//
// kms2obj
// Converts MGS2 format KMS and EVM models to OBJ and Bluepoint MDL formats.
// See command line output for detaisl.
//

//warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4200)

#include "Engine/Basics/BPEEnvironment.h"

#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "../libMDU/libMDU.h"
};
#include <math.h>
#include <direct.h>
#include <io.h>

#include "vcclr.h"
#include "Tools/Open3d/STLSupport.h"
#include "Engine/Math/CMatrix34.h"

using namespace System;
using namespace System::Xml;

//----------------------------------------------------------------------------

// see docs/mdl_fmt2.txt for details

// high bit (sign bit) is the drawing kick flag
#define DRAWING_KICK 0x8000

// dictionary is a vector
#define TEX_DICT_INITIAL_CAPACITY 256

//----------------------------------------------------------------------------
// Structs

typedef struct
{
   float x;
   float y;
   float z;
} SF3d;

typedef struct
{
   short x;
   short y;
   short z;
} SS3d;

typedef struct
{
   char caName[252];
   int id;
} SDictionaryEntry;

typedef struct 
{
   SDictionaryEntry *el;
   int size;
} SDictionary;


class CUnmanagedExportData
{
public:
   CUnmanagedExportData()
      : mpObj_fp(NULL)
      , mpMtl_fp(NULL)
   {
   }

   FILE *                  mpObj_fp;   // OBJ mesh file pointer
   FILE *                  mpMtl_fp;   // MTL material file pointer
   char                    mNameBase[256];
   char                    mSourceName[256];

   Open3d::TVector4Vector  mPoints;
   Open3d::TVector3Vector  mNormals;
   Open3d::TVector4Vector  mUvs[3];
   Open3d::TVector4Vector  mColorData;
   Open3d::TVector4Vector  mSkinWeightData;
   Open3d::TColorVector    mSkinIndexData;
};

ref class CManagedExportData
{
public:
   System::Xml::XmlDocument ^ mpDoc;
   Open3d::Scene ^            mpScene;
   Open3d::DagNode ^          mpMeshNode;
};

//----------------------------------------------------------------------------
// Globals

bool gbUseObjOutputFormat = false;

int const gkMaxMaterialCount = 4096;
char gMaterials[gkMaxMaterialCount][256];

char gFindPath[260];
char gBuiltPath[260];
char gModelName[260];
char gNameBase[260];

static bool sVerboseOutput = false;

//----------------------------------------------------------------------------
// String helpers
int find_last_of(char *str, char of)
{
   int p;
   for (p = strlen(str) - 1; p >= 0; --p)
   {
      if (str[p] == of)
      {
         break;
      }
   }
   return p;
}

//----------------------------------------------------------------------------

char *strip_extension(char *caName)
{
   // remove file extension - modifies input string
   {
      int i = strlen(caName);
      while (--i > 0)
      {
         if (caName[i] == '.')
         {
            caName[i] = 0;
            break;
         }
      }
   }
   return caName;
}

//----------------------------------------------------------------------------

void usage()
{
   fprintf(stderr, "kms2obj [-r] [-o] [-v] [kms file]\n");
   fprintf(stderr, " -r Recurse current folder and sub folders.\n");
   fprintf(stderr, " -o Output as OBJ file instead of BMDL.\n");
   fprintf(stderr, " -v Verbose output\n");
   fprintf(stderr, " [kms file] Specify file to convert if not using recursion.\n");
   fprintf(stderr, " Creates a directory for the file and puts a .obj and a .mtl file in it.\n");
   fprintf(stderr, " Use maketri for extracting the appropriate textures.\n");
}

//----------------------------------------------------------------------------

SDictionary try_read_index_file(void)
{
   int dictionaryCapacity = 0;
   FILE	*fp;
   SDictionary dictionary;
   dictionary.el = NULL;
   dictionary.size = 0;

   // attempt to read index file
   fp = fopen("idlist.txt", "rt");
   if (fp != NULL)
   {
      char caLine[512];
      char caName[260];
      int id;
      while (fgets(caLine, sizeof(caLine) - 1, fp) != NULL)
      {
         if (sscanf(caLine, "%260s => %*s : %d", caName, &id) == 2)
         {
            if (dictionary.size == dictionaryCapacity)
            {
               if (dictionaryCapacity == 0)
               {
                  dictionaryCapacity = TEX_DICT_INITIAL_CAPACITY;
                  dictionary.el = (SDictionaryEntry *) malloc(dictionaryCapacity*sizeof(SDictionaryEntry));
               }
               else
               {
                  SDictionaryEntry *newDic;
                  newDic = (SDictionaryEntry *) malloc(2*dictionaryCapacity*sizeof(SDictionaryEntry));
                  memcpy(newDic, dictionary.el, dictionaryCapacity*sizeof(SDictionaryEntry));
                  dictionary.el = newDic;
                  dictionaryCapacity *= 2;
               }
            }
            strip_extension(caName);
            strcpy(dictionary.el[dictionary.size].caName, caName);
            dictionary.el[dictionary.size].id = id;
            ++dictionary.size;
         }
      }
      fclose(fp);
   }
   return dictionary;
}

//----------------------------------------------------------------------------

char *try_get_filename_for_id(SDictionary *dic, int id)
{
   static char caFallback[256];
   int i;
   for (i = 0; i < dic->size; ++i)
   {
      SDictionaryEntry *elen = &dic->el[i];
      if (elen->id == id)
      {
         return dic->el[i].caName;
      }
   }
   sprintf(caFallback, "tex%08x.tga", id);
   return caFallback;
}

//----------------------------------------------------------------------------

void try_free_dictionary(SDictionary *dic)
{
   if (dic->el != NULL)
   {
      free(dic->el);
      dic->el = NULL;
      dic->size = 0;
   }
}

//----------------------------------------------------------------------------

SF3d SS3d_sub(SS3d *a, SS3d *b)
{
   SF3d r;
   r.x = (float)(a->x - b->x);
   r.y = (float)(a->y - b->y);
   r.z = (float)(a->z - b->z);
   return r;
}

//----------------------------------------------------------------------------

SF3d SF3d_adds(SF3d *a, SS3d *b)
{
   SF3d r;
   r.x = a->x + (float)b->x;
   r.y = a->y + (float)b->y;
   r.z = a->z + (float)b->z;
   return r;
}

//----------------------------------------------------------------------------

SF3d SF3d_addf(SF3d *a, SF3d *b)
{
   SF3d r;
   r.x = a->x + b->x;
   r.y = a->y + b->y;
   r.z = a->z + b->z;
   return r;
}

//----------------------------------------------------------------------------

SF3d SF3d_cross(SF3d *a, SF3d *b)
{
   SF3d r;
   r.x = a->y*b->z - a->z*b->y;
   r.y = b->x*a->z - b->z*a->x;
   r.z = a->x*b->y - a->y*b->x;
   return r;
}

//----------------------------------------------------------------------------

float SF3d_dot(SF3d *a, SF3d *b)
{
   return a->x*b->x + a->y*b->y + a->z*b->z;
}

//----------------------------------------------------------------------------

bool IsEvm(char *sourceName)
{
   int dotPos = find_last_of(sourceName, '.');
   if (dotPos != -1 && !_stricmp(sourceName + dotPos, ".evm"))
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

typedef struct {
   int			data_offset ;
   int			data_id ;
   int			pad[ 2 ] ;
} ZAR_LIST;

typedef struct {
   int				format_type ;	/*   */
   int				version ;		/* 0 */
   int				type ;			/* 0 */
   int				n_datas ;
   ZAR_LIST	list[ 0 ];
} ZAR_HEADER;

bool IsZms(char *sourceName)
{
   int dotPos = find_last_of(sourceName, '.');
   if (dotPos != -1 && !_stricmp(sourceName + dotPos, ".zms"))
   {
      return true;
   }

   return false;
}

std::string GetZmsKmsFilePath(const char * const zmsPath, int kmsId)
{
   char cmdlPath[FILENAME_MAX];

   char kmsIdName[9];
   sprintf( kmsIdName, "%08x", kmsId );

   strcpy( cmdlPath, zmsPath );
   strcpy( cmdlPath + strlen( zmsPath ) - 4, "\\" );
   strcat( cmdlPath, kmsIdName );
   strcat( cmdlPath, ".kms");

   return cmdlPath;
}

char zeroBuffer[1024] = {};

void ConvertSingleFileZms(char* sourceName)
{
   FILE *fp = fopen(sourceName, "rb");

   if (fp != NULL)
   {
      fseek(fp, 0, SEEK_END);
      int fileSize = ftell(fp);
      fseek(fp, 0, SEEK_SET);

      char* memory = (char*)malloc(fileSize);
      fread(memory, fileSize, 1, fp);

      ZAR_HEADER* header = (ZAR_HEADER*)memory;

      for (int i = 0; i < header->n_datas; ++i)
      {
         unsigned int kmsId = header->list[i].data_id;
         char* kmsAddress = memory + header->list[i].data_offset;
         
         char* kmsAddressEnd = memory + fileSize;

         std::string kmsPath = GetZmsKmsFilePath(sourceName, kmsId);

         std::string kmsDirectoryPath = kmsPath.substr( 0, kmsPath.find_last_of('\\') );

         _mkdir(kmsDirectoryPath.c_str());;

         FILE* kms_fp = fopen(kmsPath.c_str(), "wb");
         
         char zero[2] = { 0 };

         for( int j = 0; j < (kmsAddress - memory); ++j )
            fputc(0, kms_fp);

         fwrite(kmsAddress, kmsAddressEnd - kmsAddress, 1, kms_fp);

         fclose(kms_fp);
         
      }

      free(memory);

      fclose(fp);
   }

}

//----------------------------------------------------------------------------

bool IsKms(char *sourceName)
{
   int dotPos = find_last_of(sourceName, '.');
   if (dotPos != -1 && !_stricmp(sourceName + dotPos, ".kms"))
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

void ConvertKms(CUnmanagedExportData &unmanagedExportData, CManagedExportData ^pManagedExportData)
{
   KMS2_DEF *def;

   // also load up the idlist to map ids to textures
   SDictionary dic = try_read_index_file();
   unsigned int m, p;
   int v;
   
   int t = gbUseObjOutputFormat ? 1 : 0;  // Vertex indices start at 1 for OBJ and 0 for MDL   

   int numMaterials = 0;

   def = MDU_LoadKms2(unmanagedExportData.mSourceName);

   // each packet is a triangle strip
   // with some breaks between sub-strips indicated by the drawing kick flag in the normal.w

   if (gbUseObjOutputFormat)
   {
      fprintf(unmanagedExportData.mpObj_fp, "mtllib %s.mtl\n", unmanagedExportData.mNameBase);
   }
   
   // Check internal formats of model (has UV sets etc.)
   int const kMaxUvSetCount = 3;
   bool bHasUvSets[kMaxUvSetCount] = {0};

   if ( sVerboseOutput )
   {
      printf( "KMS: %d models, %d x_models\n", def->n_models, def->n_x_models );
   }

   for (m = 0; m < def->n_x_models; ++m)
   {
      KMS2_MDL *obj = &def->models[m];

      for (p = 0; p < obj->n_packs; ++p)
      {
         KMS2_MDLPACK *pkt = &obj->packs[p];
         for (int i = 0; i < kMaxUvSetCount; i++)
         {
            // uv are 1.3.12
            if (pkt->uvs[i] != NULL)
            {
               bHasUvSets[i] = true;
            }
         }
      }
   }

   for (m = 0; m < def->n_x_models; ++m)
   {
      KMS2_MDL *obj = &def->models[m];

      if ( sVerboseOutput )
      {
         printf( " Model %d: %d packs\n", m, obj->n_packs );
      }

      for (p = 0; p < obj->n_packs; ++p)
      {
         KMS2_MDLPACK *pkt = &obj->packs[p];

         // first write which material it's using
         char *texName = try_get_filename_for_id(&dic, pkt->tex_id[0]);
         int texIndex = 0;
         if (texName != NULL)
         {
            for (texIndex = 0; texIndex < numMaterials; ++texIndex)
            {
               if (!strcmp(gMaterials[texIndex], texName))
               {
                  break;
               }
            }
            if (texIndex == numMaterials)
            {
               if(numMaterials >= gkMaxMaterialCount)
               {
                  printf("ERROR: too many materials! %d > %d.\n", numMaterials, gkMaxMaterialCount);
                  __debugbreak();
               }

               strcpy(gMaterials[numMaterials++], texName);
               if (gbUseObjOutputFormat)
               {
                  fprintf(unmanagedExportData.mpMtl_fp, "# comment\n");
                  fprintf(unmanagedExportData.mpMtl_fp, "newmtl mat%d\nKd 1.00 1.00 1.00\nNs 1\nillum 0\n", texIndex);
                  fprintf(unmanagedExportData.mpMtl_fp, "map_Kd %s.tga\n\n", texName);
               }
            }
         }

         if (gbUseObjOutputFormat)
         {
            fprintf(unmanagedExportData.mpObj_fp, "usemtl mat%d\n", texIndex);
         }

         for (v = 0; v < pkt->n_verts; ++v)
         {
            short *vert = &pkt->verts[4*v];
            
            // the verts are 1.15.0 format.
            // the weight (vert.w) is in 1.3.12 format.
            // the bboxes in the def and obj are just for information
            float const kWeightScale = 1.0f / 4096.0f;
            float x = vert[0];
            float y = vert[1];
            float z = vert[2];
            float w = kWeightScale * vert[3];

            if (gbUseObjOutputFormat)
            {
               fprintf(unmanagedExportData.mpObj_fp, "v %f %f %f\n", x, y, z);
            }
            else
            {
               unmanagedExportData.mPoints.push_back(CVector4(x, y, z, w));
            }
         }

         for (v = 0; v < pkt->n_verts; ++v)
         {
            int const maxUvSets = gbUseObjOutputFormat ? 1 : kMaxUvSetCount;   // Only 1 set of UVs for OBJ 
            for (int i = 0; i < maxUvSets; i++)
            {
               // uv are 1.3.12
               short *uvs = pkt->uvs[i];

               // Some packets don't have UVs, add dummy UVs instead if UV stream exists in other packets
               float uv_u = 0.0f;
               float uv_v = 0.0f;
               if (uvs != NULL)
               {
                  short *uv = &uvs[2*v];
                  uv_u = uv[0]/4096.0f;
                  uv_v = uv[1]/4096.0f;
               }
               if (bHasUvSets[i])
               {
                  if (gbUseObjOutputFormat)
                  {
                     fprintf(unmanagedExportData.mpObj_fp, "vt %f %f\n", uv_u, uv_v);
                  }
                  else
                  {
                     unmanagedExportData.mUvs[i].push_back(CVector4(uv_u, uv_v, 0.0f, 0.0f));
                  }
               }
            }
         }

         for (v = 0; v < pkt->n_verts; ++v)
         {
            short *norm = &pkt->norms[4*v];
            // normals are also 1.3.12!
            float x = norm[0]/4096.0f;
            float y = norm[1]/4096.0f;
            float z = norm[2]/4096.0f;
            // normalize (generally normalized anyway...)
            float d = sqrtf(x*x + y*y + z*z);
            float nx = x / d;
            float ny = y / d;
            float nz = z / d;
            if (gbUseObjOutputFormat)
            {
               fprintf(unmanagedExportData.mpObj_fp, "vn %f %f %f\n", nx, ny, nz);
            }
            else
            {
               unmanagedExportData.mNormals.push_back(CVector3(nx, ny, nz));
            }
         }

         // tri strips with breaks
         Open3d::TIntVector vertexIndices;
         {
            int i1 = t, i2 = t + 1, i3 = t + 2;
            int clk = 1;
            for (v = 2; v < pkt->n_verts; v++)
            {
               short *verts = pkt->verts;
               short *norm = &pkt->norms[4*v];
               short w = norm[3];
               if( (w & DRAWING_KICK) == 0 )
               {
                  SS3d *v1v = (SS3d *)&verts[4*(i1-t)];
                  SS3d *v2v = (SS3d *)&verts[4*(i2-t)];
                  SS3d *v3v = (SS3d *)&verts[4*(i3-t)];
                  SF3d e1 = SS3d_sub(v2v, v1v);
                  SF3d e2 = SS3d_sub(v3v, v1v);
                  SF3d nc = SF3d_cross(&e1, &e2);
                  SF3d n;
                  n.x = norm[0]/4096.0f;
                  n.y = norm[1]/4096.0f;
                  n.z = norm[2]/4096.0f;

                  bool const bTriFlipped = (SF3d_dot(&n, &nc) <= 0.0f);
                  if (gbUseObjOutputFormat)
                  {
                     if (!bTriFlipped)
                     {
                        // triangle is wound the right way
                        fprintf(unmanagedExportData.mpObj_fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", i1, i1, i1, i2, i2, i2, i3, i3, i3);
                     }
                     else
                     {
                        // triangle is wound the wrong way!
                        fprintf(unmanagedExportData.mpObj_fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", i1, i1, i1, i3, i3, i3, i2, i2, i2);
                     }
                  }
                  else
                  {
                     if (!bTriFlipped)
                     {
                        vertexIndices.push_back(i1);
                        vertexIndices.push_back(i2);
                        vertexIndices.push_back(i3);
                     }
                     else
                     {
                        vertexIndices.push_back(i1);
                        vertexIndices.push_back(i3);
                        vertexIndices.push_back(i2);
                     }
                  }
               }
               if (clk)
               {
                  i1 = i3;
                  clk = 0;
               }
               else
               {
                  i2 = i3;
                  clk = 1;
               }
               i3++;
            }
            t += pkt->n_verts;
         }
         if (!gbUseObjOutputFormat)
         {
            // Submesh info for MDL
            Open3d::Submesh ^ pSubMesh = pManagedExportData->mpScene->CreateSubmesh();

            pSubMesh->Material = gcnew System::String(texName);
            pSubMesh->Node->SetAttribute("unitIdx", m.ToString());   // Model index
            pSubMesh->Node->SetAttribute("packetIdx", p.ToString()); // Packet index

            if (!vertexIndices.empty())
            {
               {
                  Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
                  pIndices->Inputs = gcnew array<String^>{ "Points", "Normals" };
                  pIndices->SetIndices(vertexIndices);
                  pSubMesh->Add(pIndices);
               }

               // Multi UV sets might need further investigation
               if (!unmanagedExportData.mUvs[0].empty())
               {
                  Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
                  pIndices->Inputs = gcnew array<String^>{ "UV0" };
                  pIndices->SetIndices(vertexIndices);
                  pSubMesh->Add(pIndices);
               }
               
               if (!unmanagedExportData.mUvs[1].empty())
               {
                  Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
                  pIndices->Inputs = gcnew array<String^>{ "UV1" };
                  pIndices->SetIndices(vertexIndices);
                  pSubMesh->Add(pIndices);
               }

               if (!unmanagedExportData.mUvs[2].empty())
               {
                  Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
                  pIndices->Inputs = gcnew array<String^>{ "UV2" };
                  pIndices->SetIndices(vertexIndices);
                  pSubMesh->Add(pIndices);
               }
           }
   #if 0
            //TODO
            if( hasSkinningData && !vertexIndices.empty() )
            {
               Open3d::IndexArray^ pIndices = md->mpScene->CreateIndexArray();
               pIndices->Inputs = gcnew array<String^>{ "SkinIndex", "SkinWeight" };
               pIndices->SetIndices(vertexIndices);
               pSubMesh->Add(pIndices);
            }
   #endif

            pManagedExportData->mpMeshNode->Add(pSubMesh);
         }
      }
   }

   MDU_Free(def);
}

//----------------------------------------------------------------------------

void ConvertEvm(CUnmanagedExportData &unmanagedExportData, CManagedExportData ^pManagedExportData)
{
   EVM_DEF* def = MDU_LoadEvm(unmanagedExportData.mSourceName);

   System::String^ materialName = gcnew System::String("Not implemented");

   int const DG_EVMTYPE_LARGE = 1;
   float const posScale = (def->type & DG_EVMTYPE_LARGE) ? 1.0f : (1.0f / 16.0f);

   int t = gbUseObjOutputFormat ? 1 : 0;  // Vertex indices start at 1 for OBJ and 0 for MDL

   // Check internal formats of model (has UV sets etc.)
   int const kMaxUvSetCount = 3;
   bool bHasUvSets[kMaxUvSetCount] = {0};
   for (int p = 0; p < def->n_packs; ++p)
   {
      EVM_PACK* pkt = def->packet + p;
      for (int i = 0; i < kMaxUvSetCount; i++)
      {
         // uv are 1.3.12
         if (pkt->uvs[i] != NULL)
         {
            bHasUvSets[i] = true;
         }
      }     
   }


   for (int p = 0; p < def->n_packs; ++p)
   {
      EVM_PACK* pkt = def->packet + p;

      for (int v = 0; v < pkt->n_verts; ++v)
      {
         short *vert = (short*)pkt->verts;

         // the verts are 1.15.0 or 1.11.4 format.
         float x = posScale * vert[4*v + 0];
         float y = posScale * vert[4*v + 1];
         float z = posScale * vert[4*v + 2];

         if (gbUseObjOutputFormat)
         {
            fprintf(unmanagedExportData.mpObj_fp, "v %f %f %f\n", x, y, z);
         }
         else
         {
            unmanagedExportData.mPoints.push_back(CVector4(x, y, z, 1.0f));
         }
      }

      for (int v = 0; v < pkt->n_verts; ++v)
      {
         int const maxUvSets = gbUseObjOutputFormat ? 1 : kMaxUvSetCount;   // Only 1 set of UVs for OBJ 
         for (int i = 0; i < maxUvSets; i++)
         {
            if (bHasUvSets[i])
            {
               // uv are 1.3.12
               short *uvs = (short*)pkt->uvs[i];

               // Some packets don't have UVs, add dummy UVs instead if UV stream exists in other packets
               float uv_x = 0.0f;
               float uv_y = 0.0f;
               float uv_z = 0.0f;
               float uv_w = 0.0f;
               
               if (uvs != NULL)
               {
                  short *uv = &uvs[4*v];
                  uv_x = uv[0]/4096.0f;
                  uv_y = uv[1]/4096.0f;
                  uv_z = uv[2]/4096.0f;
                  uv_w = uv[3]/4096.0f;
               }

               if (gbUseObjOutputFormat)
               {
                  fprintf(unmanagedExportData.mpObj_fp, "vt %f %f\n", uv_x, uv_y);
               }
               else
               {
                  unmanagedExportData.mUvs[i].push_back(CVector4(uv_x, uv_y, uv_z, uv_w));
               }
            }
         }
      }

      for (int v = 0; v < pkt->n_verts; ++v)
      {
         short *norm = (short*)pkt->norms;
         norm += 4*v;
         // normals are 1.3.12.
         float x = norm[0]/4096.0f;
         float y = norm[1]/4096.0f;
         float z = norm[2]/4096.0f;

         if (gbUseObjOutputFormat)
         {
            fprintf(unmanagedExportData.mpObj_fp, "vn %f %f %f\n", x, y, z);
         }
         else
         {
            unmanagedExportData.mNormals.push_back(CVector3(x, y, z));
         }
      }

      // Extract weight data
      {
         unsigned char* inputIndices = pkt->mat_id;

         for (int v = 0; v < pkt->n_verts; ++v)
         {
            unsigned char* inputWeights = (unsigned char*)pkt->weight;
            inputWeights += 8*v;
            
            unsigned char* localJointOffsets = inputWeights + 4;

            real32 weights[] = { 1.0f, 0.0f, 0.0f, 0.0f };
            int indices[] = {0, 0, 0, 0};

            int outJointIdx = 0;

            for( int idx = 0; idx < pkt->n_mats; ++idx )
            {
               int const localJointIndex = localJointOffsets[idx] / 4;
               int const jointIndex = inputIndices[localJointIndex];
               real32 const jointWeight = inputWeights[idx] / 128.0f;
               if( jointWeight > 0.0f )
               {
                  indices[outJointIdx] = jointIndex;
                  weights[outJointIdx] = jointWeight;
                  outJointIdx++;
               }
            }

            unmanagedExportData.mSkinWeightData.push_back(CVector4(weights[0], weights[1], weights[2], weights[3]));
            unmanagedExportData.mSkinIndexData.push_back(CColor(indices[0], indices[1], indices[2], indices[3]));
         }
      }

      // tri strips with breaks
      Open3d::TIntVector vertexIndices;
      {

         int i1 = t, i2 = t + 1, i3 = t + 2;
         int clk = 1;
         for (int v = 2; v < pkt->n_verts; v++)
         {
            short *verts = (short*)pkt->verts;
            short *norm = (short*)pkt->norms;
            norm += 4*v;
            
            short const f = verts[4*v + 3];

            if ((f & DRAWING_KICK) == 0)
            {
               SS3d *v1v = (SS3d *)&verts[4*(i1-t)];
               SS3d *v2v = (SS3d *)&verts[4*(i2-t)];
               SS3d *v3v = (SS3d *)&verts[4*(i3-t)];
               SF3d e1 = SS3d_sub(v2v, v1v);
               SF3d e2 = SS3d_sub(v3v, v1v);
               SF3d nc = SF3d_cross(&e1, &e2);
               SF3d n;
               n.x = norm[0]/4096.0f;
               n.y = norm[1]/4096.0f;
               n.z = norm[2]/4096.0f;
               bool const bTriFlipped = (SF3d_dot(&n, &nc) <= 0.0f);
               if (gbUseObjOutputFormat)
               {
                  if (!bTriFlipped)
                  {
                     // triangle is wound the right way
                     fprintf(unmanagedExportData.mpObj_fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", i1, i1, i1, i2, i2, i2, i3, i3, i3);
                  }
                  else
                  {
                     // triangle is wound the wrong way!
                     fprintf(unmanagedExportData.mpObj_fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", i1, i1, i1, i3, i3, i3, i2, i2, i2);
                  }
               }
               else
               {
                  if (!bTriFlipped)
                  {
                     vertexIndices.push_back(i1);
                     vertexIndices.push_back(i2);
                     vertexIndices.push_back(i3);
                  }
                  else
                  {
                     vertexIndices.push_back(i1);
                     vertexIndices.push_back(i3);
                     vertexIndices.push_back(i2);
                  }
               }
            }
            if (clk)
            {
               i1 = i3;
               clk = 0;
            }
            else
            {
               i2 = i3;
               clk = 1;
            }
            i3++;
         }
         t += pkt->n_verts;
      }
      if (!gbUseObjOutputFormat)
      {
         // Submesh info for MDL
         Open3d::Submesh ^ pSubMesh = pManagedExportData->mpScene->CreateSubmesh();

         pSubMesh->Material = materialName;
         int const unitIdx = 0;
         pSubMesh->Node->SetAttribute("unitIdx", unitIdx.ToString());   // Model index (always zero for EVM format as only one model)
         pSubMesh->Node->SetAttribute("packetIdx", p.ToString()); // Packet index

         if (!vertexIndices.empty())
         {
            {
               Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
               pIndices->Inputs = gcnew array<String^>{ "Points", "Normals", "SkinIndex", "SkinWeight" };
               pIndices->SetIndices(vertexIndices);
               pSubMesh->Add(pIndices);
            }

            // Multi UV sets might need further investigation
            if (!unmanagedExportData.mUvs[0].empty())
            {
               Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
               pIndices->Inputs = gcnew array<String^>{ "UV0" };
               pIndices->SetIndices(vertexIndices);
               pSubMesh->Add(pIndices);
            }
            
            if (!unmanagedExportData.mUvs[1].empty())
            {
               Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
               pIndices->Inputs = gcnew array<String^>{ "UV1" };
               pIndices->SetIndices(vertexIndices);
               pSubMesh->Add(pIndices);
            }

            if (!unmanagedExportData.mUvs[2].empty())
            {
               Open3d::IndexArray^ pIndices = pManagedExportData->mpScene->CreateIndexArray();
               pIndices->Inputs = gcnew array<String^>{ "UV2" };
               pIndices->SetIndices(vertexIndices);
               pSubMesh->Add(pIndices);
            }
         }

         pManagedExportData->mpMeshNode->Add(pSubMesh);
      }
   }

   MDU_Free(def);
}

//----------------------------------------------------------------------------

void ConvertSingleFileToOBJ(CUnmanagedExportData &unmanagedExportData, CManagedExportData ^pManagedExportData)
{
   {
      char fileName[260];
      sprintf(fileName, "%s.obj", unmanagedExportData.mNameBase);
      unmanagedExportData.mpObj_fp = fopen(fileName, "wt");
      sprintf(fileName, "%s.mtl", unmanagedExportData.mNameBase);
      unmanagedExportData.mpMtl_fp = fopen(fileName, "wt");
   }

   if ((unmanagedExportData.mpObj_fp != NULL) & (unmanagedExportData.mpMtl_fp != NULL))
   {
      if (IsKms(unmanagedExportData.mSourceName))
      {
         ConvertKms(unmanagedExportData, pManagedExportData);
      }
      else if (IsEvm(unmanagedExportData.mSourceName))
      {
         ConvertEvm(unmanagedExportData, pManagedExportData);
      }
      else
      {
         fclose(unmanagedExportData.mpMtl_fp);
         fclose(unmanagedExportData.mpObj_fp);
         return;
      }
      fclose(unmanagedExportData.mpMtl_fp);
      fclose(unmanagedExportData.mpObj_fp);

      printf("Converted: %s\n", unmanagedExportData.mSourceName);
   }
   else
   {
      printf("Problem writing output files for %s.\n", unmanagedExportData.mNameBase);
   }
}

//----------------------------------------------------------------------------

void ConvertSingleFileToMDL(CUnmanagedExportData &unmanagedExportData, CManagedExportData ^pManagedExportData)
{
   pManagedExportData->mpDoc = gcnew XmlDocument();
   pManagedExportData->mpDoc->AppendChild( pManagedExportData->mpDoc->CreateXmlDeclaration( "1.0", nullptr, nullptr ) );

   pManagedExportData->mpScene = Open3d::Scene::Create( pManagedExportData->mpDoc );
   pManagedExportData->mpDoc->AppendChild( pManagedExportData->mpScene );
   pManagedExportData->mpMeshNode = pManagedExportData->mpScene->CreateDagNode("MDL_Root");
   pManagedExportData->mpScene->Add(pManagedExportData->mpMeshNode);

   {
      Open3d::Transform^ pTransform = pManagedExportData->mpScene->CreateTransform();
      pTransform->SetTransform(CMatrix34::Identity());
      pManagedExportData->mpMeshNode->Add(pTransform);
   }

   // Add mesh data
   if (IsKms(unmanagedExportData.mSourceName))
   {
      ConvertKms(unmanagedExportData, pManagedExportData);
   }
   else if (IsEvm(unmanagedExportData.mSourceName))
   {
      ConvertEvm(unmanagedExportData, pManagedExportData);
   }

   //Finalize data
   if( !unmanagedExportData.mPoints.empty() )
   {
      Open3d::DataArray ^ pPoints = pManagedExportData->mpScene->CreateDataArray("Points");
      pPoints->SetData( unmanagedExportData.mPoints );
      pManagedExportData->mpMeshNode->Add(pPoints);
   }
   if( !unmanagedExportData.mNormals.empty() )
   {
      Open3d::DataArray ^ pNormals = pManagedExportData->mpScene->CreateDataArray("Normals");
      pNormals->SetData( unmanagedExportData.mNormals );
      pManagedExportData->mpMeshNode->Add(pNormals);
   }
   if( !unmanagedExportData.mUvs[0].empty() )
   {
      Open3d::DataArray ^ pUV = pManagedExportData->mpScene->CreateDataArray("UV0");
      pUV->SetData( unmanagedExportData.mUvs[0] );
      pManagedExportData->mpMeshNode->Add(pUV);
   }
   if( !unmanagedExportData.mUvs[1].empty() )
   {
      Open3d::DataArray ^ pUV = pManagedExportData->mpScene->CreateDataArray("UV1");
      pUV->SetData( unmanagedExportData.mUvs[1] );
      pManagedExportData->mpMeshNode->Add(pUV);
   }
   if( !unmanagedExportData.mUvs[2].empty() )
   {
      Open3d::DataArray ^ pUV = pManagedExportData->mpScene->CreateDataArray("UV2");
      pUV->SetData( unmanagedExportData.mUvs[2] );
      pManagedExportData->mpMeshNode->Add(pUV);
   }
   if( !unmanagedExportData.mColorData.empty() )
   {
      // AndyO: Not investigated color data yet for MGS2...
      // The color array is sparse, so make sure we fill any voids with a NOP color
      unmanagedExportData.mColorData.resize(unmanagedExportData.mPoints.size(), CVector4(1.0f, 1.0f, 1.0f, 1.0f));

      Open3d::DataArray ^ pColors0 = pManagedExportData->mpScene->CreateDataArray("VertColor0");
      pColors0->SetData(unmanagedExportData.mColorData);
      pManagedExportData->mpMeshNode->Add(pColors0);
   }

   if( !unmanagedExportData.mSkinIndexData.empty() )
   {
      Open3d::DataArray ^ pSkinIndices = pManagedExportData->mpScene->CreateDataArray("SkinIndex");
      pSkinIndices->SetData(unmanagedExportData.mSkinIndexData);
      pManagedExportData->mpMeshNode->Add(pSkinIndices);
   }

   if( !unmanagedExportData.mSkinWeightData.empty() )
   {
      Open3d::DataArray ^ pSkinWeights = pManagedExportData->mpScene->CreateDataArray("SkinWeight");
      pSkinWeights->SetData(unmanagedExportData.mSkinWeightData);
      pManagedExportData->mpMeshNode->Add(pSkinWeights);
   }

   try
   {
      char fileName[260];
      sprintf(fileName, "%s.bmdl", unmanagedExportData.mNameBase);
      pManagedExportData->mpDoc->Save( gcnew String(fileName) );
      printf("Converted: %s\n", unmanagedExportData.mSourceName);
   }
   catch(System::Exception^ )
   {
      //Ignore all exceptions even though we really just mean to ignore file access exceptions.
   }
}

//----------------------------------------------------------------------------

int file_exists(char *name)
{
   FILE *fp = fopen(name, "rb");
   if (fp)
   {
      fclose(fp);
      return TRUE;
   }
   return FALSE;
}

//----------------------------------------------------------------------------

void ConvertSingleFile(char *sourceName)
{
   if (!file_exists(sourceName))
   {
      printf("File not found.\n");
      return;
   }
   if( IsZms(sourceName) )
   {
      ConvertSingleFileZms(sourceName);
      return;
   }

   if (!IsKms(sourceName) & !IsEvm(sourceName))
   {
      // Not a valid extension type.
      return;
   }

   CManagedExportData ^ pManagedExportData = gcnew CManagedExportData();
   CUnmanagedExportData unmanagedExportData;

   strcpy(unmanagedExportData.mSourceName, sourceName);
   // remove extension
   strcpy(unmanagedExportData.mNameBase, sourceName);
   unmanagedExportData.mNameBase[strlen(sourceName) - 4] = 0;

   if (gbUseObjOutputFormat)
   {
      ConvertSingleFileToOBJ(unmanagedExportData, pManagedExportData);
   }
   else
   {
      ConvertSingleFileToMDL(unmanagedExportData, pManagedExportData);
   }
}


//----------------------------------------------------------------------------

void ConvertRecursively()
{
   struct _finddata32_t finddata;
   int handle = 0;
   {
      sprintf_s(gFindPath, 260, "%s\\*.*", gBuiltPath);
      handle = _findfirst32(gFindPath, &finddata);
   }
   if (handle != -1)
   {
      do
      {
         if (!(finddata.attrib & _A_SUBDIR))
         {
            sprintf_s(gModelName, 260, "%s\\%s", gBuiltPath, finddata.name);
            ConvertSingleFile(gModelName);
         }
         else if (_stricmp(finddata.name, ".") && _stricmp(finddata.name, ".."))
         {
            int pathLen = strlen(gBuiltPath);
            sprintf_s(&gBuiltPath[pathLen], 260 - pathLen, "\\%s", finddata.name);
            ConvertRecursively();
            // reset path
            gBuiltPath[pathLen] = 0;
         }
      }
      while (_findnext32(handle, &finddata) != -1);
      _findclose(handle);
   }
}

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   if (argc < 2)
   {
      usage();
      return -1;
   }
   bool bRecurse = false;
   for (int i = 1; i < argc; i++)
   {
      int len = strlen(argv[i]);
      if (argv[i][0] == '-')
      {
         if (len < 2)
         {
            usage();
            return -1;
         }
         switch (toupper(argv[i][1]))
         {
         case 'R':
            bRecurse = true;
            _getcwd(gBuiltPath, 260);
            break;
         case 'V':
            sVerboseOutput = true;
            break;
         case 'O':
            gbUseObjOutputFormat = true;
            break;
         default:
            usage();
            return -1;
         }
      }
      else
      {
         // Single file
         if ((i != (argc - 1)) || !IsKms(argv[i]) && !IsEvm(argv[i]) && !IsZms(argv[i]) )
         {
            usage();
            return -1;
         }
         strcpy(gModelName, argv[i]);
      }
   }
   if (!bRecurse)
   {
      ConvertSingleFile(gModelName);
   }
   else
   {
      ConvertRecursively();
   }
}

//----------------------------------------------------------------------------
