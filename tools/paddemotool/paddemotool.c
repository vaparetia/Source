#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <windows.h>
#include <sys/stat.h>
#include <time.h>

/* mgs_type.h */

typedef unsigned __int8    u_char;
typedef unsigned __int16   u_short;
typedef unsigned __int32   u_int;
typedef struct { float vx, vy, vz, vw; } FVECTOR;

/* libgv.h */

#define PAD_U     (0x00001000)
#define PAD_D     (0x00004000)
#define PAD_L     (0x00008000)
#define PAD_R     (0x00002000)
#define PAD_A     (0x00000020)
#define PAD_B     (0x00000040)
#define PAD_X     (0x00000010)
#define PAD_Y     (0x00000080)

#define PAD_L1    (0x00000004)
#define PAD_R1    (0x00000008)
#define PAD_L2    (0x00000001)
#define PAD_R2    (0x00000002)

#define PAD_STA   (0x00000800)
#define PAD_SEL   (0x00000100)

#define PAD_AL    (0x00000200)
#define PAD_AR    (0x00000400)

/* Armature: incremented version number! */

#define PADDEMO_VERSION 3

#define PADDEMO_MAGIC 0xe6

enum
{
   SV_PADDEMO_COMMAND_SWITCHWEAPON,
   SV_PADDEMO_COMMAND_SWITCHITEM
};

/* paddemo.c */

/* padrec.c と同じにすること */
typedef struct
{
   u_char      magic;
   u_char      version;
   u_short     length;
   u_int       flag;
   u_char      *type;
   u_short     *button_le;

   u_char      *analog;
   u_short     *press_button_le;
   u_char      *press_data;   
   int         start_turn;

   // size is 32 to this point
   // FVECTOR is 16 byte aligned
   FVECTOR     start_pos;
   int         start_time;

   // size is 52 to this point
   // SV_PAD_HEADER is 16 byte aligned
   u_char      *command;
   char         pad[8];
}
SV_PAD_HEADER;

enum
{
   SV_TYPE_NONE                  = 0x00,
   SV_TYPE_BUTTON                = 0x01,
   SV_TYPE_ANALOG_L              = 0x02,
   SV_TYPE_ANALOG_R              = 0x04,   
   SV_TYPE_PRESS                 = 0x08,
   SV_TYPE_BUTTON_CHANGED        = 0x10,
   SV_TYPE_PRESS_BUTTON_CHANGED  = 0x20,
   SV_TYPE_PAUSE_MENU            = 0x40,
   SV_TYPE_COMMAND               = 0x80
};

typedef struct
{
   int mask;
   char *name;
}
SPadPress;

// these are in the order that padrec.c writes them out!
SPadPress skPadPress[] =
{
   { PAD_U, "U" },
   { PAD_D, "D" },
   { PAD_L, "L" },
   { PAD_R, "R" },
   { PAD_A, "A" },
   { PAD_B, "B" },
   { PAD_X, "X" },
   { PAD_Y, "Y" },
   { PAD_L1, "L1" },
   { PAD_R1, "R1" },
   { PAD_L2, "L2" },
   { PAD_R2, "R2" },
   { PAD_STA, "START" },
   { PAD_SEL, "SELECT" },
   { PAD_AL, "AL"},
   { PAD_AR, "AR"},
};

static void replace_extension(char *filename, char *ext)
{
   sprintf(filename + strlen(filename) - 3, ext);
}

static void dump_paddemo(char *filename)
{
   FILE *f = fopen(filename, "rb");
   int er1 = fseek(f, 0, SEEK_END);
   int len = ftell(f);
   int er2 = fseek(f, 0, SEEK_SET);
   SV_PAD_HEADER *head = (SV_PAD_HEADER *)malloc(len);
   int er3 = fread(head, len, 1, f);
   int er4 = fclose(f);
   u_short cur_button = 0, cur_press_button = 0;
   u_short button;
   int i, count;
   u_int vx, vy, vz, vw;

   if (head->magic != PADDEMO_MAGIC)
   {
      printf("%s magic number is wrong! (%02x/%02x)\n", filename, head->magic, PADDEMO_MAGIC);
      return;
   }
   if (head->version < 1 || head->version > PADDEMO_VERSION)
   {
      printf("%s version is wrong! (%d)\n", filename, head->version);
      return;
   }
   if (head->version == 1)
   {
      printf("Suspicious!\n");
   }
   
   // replace the extension
   replace_extension(filename, "txt");
   f = fopen(filename, "wt");

   // first line is the other info
   vx = *((u_int *)&head->start_pos.vx);
   vy = *((u_int *)&head->start_pos.vy);
   vz = *((u_int *)&head->start_pos.vz);
   vw = *((u_int *)&head->start_pos.vw);
   fprintf(f, "%02x %02x %04x %d %08x %08x %08x %08x %d", head->magic, head->version, head->flag, head->start_turn, vx, vy, vz, vw, head->start_time);

   /* relocate pointers */
   head->type              = ( u_char *  )( (u_char *)head + ( int )head->type );
   head->button_le         = ( u_short * )( (u_char *)head + ( int )head->button_le );
   head->analog            = ( u_char *  )( (u_char *)head + ( int )head->analog );
   head->press_button_le   = ( u_short * )( (u_char *)head + ( int )head->press_button_le );
   head->press_data        = ( u_char *  )( (u_char *)head + ( int )head->press_data );
   head->command           = ( u_char *  )( (u_char *)head + ( int )head->command );

   for (count = 0; count < head->length; ++count)
   {
      u_char type = *head->type++;
      char comment[128] = { 0 };
      if ( type & SV_TYPE_BUTTON )
      {
         if ( type & SV_TYPE_BUTTON_CHANGED )
         {
            cur_button = *head->button_le++;
         }
         button = cur_button;
         fprintf(f, "button %04x ", button);
         sprintf(comment, "button ");
         for (i = 0; i < 16; ++i)
         {
            if (button & skPadPress[i].mask)
            {
               strcat(comment, skPadPress[i].name);
               strcat(comment, " ");
            }
         }
      }
      if ( type & SV_TYPE_ANALOG_L )
      {
         u_char a1 = *head->analog++;
         u_char a2 = *head->analog++;
         fprintf(f, "analogl %d %d ", a1, a2);
      }
      if ( type & SV_TYPE_ANALOG_R )
      {
         u_char a1 = *head->analog++;
         u_char a2 = *head->analog++;
         fprintf(f, "analogr %d %d ", a1, a2);
      }
      if ( type & SV_TYPE_PRESS )
      {
         if ( type & SV_TYPE_PRESS_BUTTON_CHANGED )
         {
            cur_press_button = *head->press_button_le++;
         }
         button = cur_press_button;
         fprintf(f, "pressure %04x ", button);
         strcat(comment, "pressure ");
         for (i = 0; i < 16; ++i)
         {
            if (button & skPadPress[i].mask)
            {
               u_char pressure = *head->press_data++;
               fprintf(f, "%d ", pressure);
               strcat(comment, skPadPress[i].name);
               strcat(comment, " ");
            }
         }
      }
      if ( type & SV_TYPE_COMMAND )
      {
         int command = *head->command++;
         int command_data = *head->command++;
         fprintf(f, "command %d %d ", command, command_data);
         switch (command)
         {
         case SV_PADDEMO_COMMAND_SWITCHWEAPON:
            strcat(comment, "switch weapon ");
            break;
         case SV_PADDEMO_COMMAND_SWITCHITEM:
            strcat(comment, "switch item ");
            break;
         }
      }
      if (strlen(comment) > 0)
      {
         fprintf(f, " // ");
         fprintf(f, comment);
      }
      fprintf(f, "\n");
   }
   fclose(f);

   free(head);
}

char *tokens[64];

int string_tokenize(char *buf)
{
   int i, n = 0, len = strlen(buf);
   char *tok = buf;
   for (i = 0; i < len; ++i)
   {
      if (buf[i] == ' ')
      {
         tokens[n++] = tok;
         buf[i] = 0;
         tok = &buf[i] + 1;
      }
   }
   return n;
}

static int align4( int size )
{
   return (size + 3) & 0xFFFFFFFC;
}

static void parse_paddemo(char *filename)
{
   FILE *f = fopen(filename, "rt");
   int i, count = 0;
   int maxsize;
   SV_PAD_HEADER head;
   SV_PAD_HEADER *pHead;
   char buf[512];
   int cur_button = 0;
   int cur_press_button = 0;
   int button_count = 0;
   int press_button_count = 0;
   int analog_count = 0;
   int press_data_count = 0;
   int command_count = 0;
   u_int button;
   // count the number of lines
   while (!feof(f))
   {
      fgets(buf, 511, f);
      count++;
   }
   // skip the header
   count--;
   // each array can't possibly be longer than the text file
   maxsize = ftell(f);
   head.type = malloc(count);
   head.button_le = malloc(maxsize);
   head.analog = malloc(maxsize);
   head.press_button_le = malloc(maxsize);
   head.press_data = malloc(maxsize);
   head.command = malloc(maxsize);
   // rewind
   fseek(f, 0, SEEK_SET);
   // first line is the header
   fscanf(f, "%02x %02x %04x %d %08x %08x %08x %08x %d", &head.magic, &head.version, &head.flag, &head.start_turn, &head.start_pos.vx, &head.start_pos.vy, &head.start_pos.vz, &head.start_pos.vw, &head.start_time);
   head.length = count;

   for (i = 0; i < count; ++i)
   {
      head.type[i] = 0;
      fgets(buf, 511, f);
      // remove the cr
      buf[strlen(buf) - 1] = 0;
      if (strlen(buf) > 0)
      {
         // split it into tokens
         int j, n = string_tokenize(buf);
         for (j = 0; j < n; ++j)
         {
            if (!strcmp(tokens[j], "//"))
            {
               // stop at the comment
               break;
            }
            else if (!strcmp(tokens[j], "button"))
            {
               head.type[i] |= SV_TYPE_BUTTON;
               sscanf(tokens[++j], "%04x", &button);
               if (button != cur_button)
               {
                  cur_button = button;
                  head.type[i] |= SV_TYPE_BUTTON_CHANGED;
                  head.button_le[button_count++] = button;
               }
            }
            else if (!strcmp(tokens[j], "pressure"))
            {
               int k;
               head.type[i] |= SV_TYPE_PRESS;
               sscanf(tokens[++j], "%04x", &button);
               if (button != cur_press_button)
               {
                  cur_press_button = button;
                  head.type[i] |= SV_TYPE_PRESS_BUTTON_CHANGED;
                  head.press_button_le[press_button_count++] = button;
               }
               for (k = 0; k < 16; ++k)
               {
                  if (button & skPadPress[k].mask)
                  {
                     int a;
                     sscanf(tokens[++j], "%d", &a);
                     head.press_data[press_data_count++] = a;
                  }
               }
            }
            else if (!strcmp(tokens[j], "analogl"))
            {
               int a;
               head.type[i] |= SV_TYPE_ANALOG_L;
               sscanf(tokens[++j], "%d", &a);
               head.analog[analog_count++] = a;
               sscanf(tokens[++j], "%d", &a);
               head.analog[analog_count++] = a;
            }
            else if (!strcmp(tokens[j], "analogr"))
            {
               int a;
               head.type[i] |= SV_TYPE_ANALOG_R;
               sscanf(tokens[++j], "%d", &a);
               head.analog[analog_count++] = a;
               sscanf(tokens[++j], "%d", &a);
               head.analog[analog_count++] = a;
            }
            else if (!strcmp(tokens[j], "command"))
            {
               int a;
               head.type[i] |= SV_TYPE_COMMAND;
               // add to the command stream
               sscanf(tokens[++j], "%d", &a); // command
               head.command[command_count++] = a;
               sscanf(tokens[++j], "%d", &a); // command data
               head.command[command_count++] = a;
            }
         }
      }
   }
   fclose(f);
   // now reallocate enough to fit everything, copy it over, and write it out
   {
      int size = align4(sizeof(SV_PAD_HEADER)) 
         + align4(sizeof( u_char)*count)
         + align4(sizeof(u_short)*button_count)
         + align4(sizeof( u_char)*analog_count)		
         + align4(sizeof(u_short)*press_button_count)
         + align4(sizeof( u_char)*press_data_count)
         + align4(sizeof( u_char)*command_count);
      size = (size+15) & 0xFFFFFFF0;
      pHead = (SV_PAD_HEADER *)malloc(size);
      memset(pHead, 0, size);

      pHead->magic = head.magic;
      pHead->version = 3;
      pHead->flag = head.flag;
      pHead->length = head.length;
      pHead->start_turn = head.start_turn;
      pHead->start_pos = head.start_pos;
      pHead->start_time = head.start_time;

      pHead->type             = ( u_char *) pHead                    + align4(sizeof(SV_PAD_HEADER));
      pHead->button_le        = (u_short *)(pHead->type              + align4(sizeof( u_char)*count));
      pHead->analog           = ( u_char *) pHead->button_le         + align4(sizeof(u_short)*button_count);
      pHead->press_button_le  = (u_short *)(pHead->analog            + align4(sizeof( u_char)*analog_count));
      pHead->press_data       = ( u_char *) pHead->press_button_le   + align4(sizeof(u_short)*press_button_count);
      pHead->command          = ( u_char *) pHead->press_data        + align4(sizeof( u_char)*press_data_count);

      memcpy(pHead->type,              head.type,              sizeof( u_char)*count);
      memcpy(pHead->button_le,         head.button_le,         sizeof(u_short)*button_count);
      memcpy(pHead->analog,            head.analog,            sizeof( u_char)*analog_count);
      memcpy(pHead->press_button_le,   head.press_button_le,   sizeof(u_short)*press_button_count);
      memcpy(pHead->press_data,        head.press_data,        sizeof( u_char)*press_data_count);
      memcpy(pHead->command,           head.command,           sizeof( u_char)*command_count);

      pHead->type             =             (u_char *)pHead->type             - (u_int)pHead;
      pHead->button_le        = (u_short *)((u_char *)pHead->button_le        - (u_int)pHead);
      pHead->analog           =             (u_char *)pHead->analog           - (u_int)pHead;
      pHead->press_button_le  = (u_short *)((u_char *)pHead->press_button_le  - (u_int)pHead);
      pHead->press_data       =             (u_char *)pHead->press_data       - (u_int)pHead;
      pHead->command          =             (u_char *)pHead->command          - (u_int)pHead;

      // write out the new file
      replace_extension(filename, "row");
      f = fopen(filename, "wb");
      if (f != NULL)
      {
         fwrite(pHead, size, 1, f);
         fclose(f);
      }
      else
      {
         printf("%s is probably not checked out.\n", filename);
      }

      free(pHead);
   }

   free(head.type);
   free(head.button_le);
   free(head.analog);
   free(head.press_button_le);
   free(head.press_data);
   free(head.command);
}

static int string_ends_with(char *a, char *b)
{
   int ends_with = 0;
   if (strlen(a) >= strlen(b))
   {
      ends_with = !strcmp(a + strlen(a) - strlen(b), b);
   }
   return ends_with;
}

static int is_paddemo(char *filename)
{
   FILE *f = fopen(filename, "rb");
   u_char magic;
   fread(&magic, 1, 1, f);
   fclose(f);
   return (magic == PADDEMO_MAGIC);
}

static FILETIME get_modified_time(char *filename)
{
   WIN32_FILE_ATTRIBUTE_DATA attrib;
   GetFileAttributesExA(filename, GetFileExInfoStandard, &attrib);
   return attrib.ftLastWriteTime;
}

static int compare_modified_times(char *a, char *b)
{
   FILETIME at = get_modified_time(a);
   FILETIME bt = get_modified_time(b);

   if (at.dwHighDateTime > bt.dwHighDateTime)
   {
      return 1;
   }
   else if (at.dwHighDateTime < bt.dwHighDateTime)
   {
      return -1;
   }
   return at.dwLowDateTime - bt.dwLowDateTime;
}

static int file_exists(char *filename)
{
   FILE *f = fopen(filename, "r");
   if (f)
   {
      fclose(f);
      return 1;
   }
   return 0;
}

typedef enum
{
   kDump,
   kCompile,
   kFind
}
EDumpOrCompile;

static int gHashToFind;

int GV_StrCode( char *string )
{
   unsigned char c;
   unsigned char *p;
   unsigned int id, mask;

   p = ( unsigned char * )string;
   id = 0;
   mask = 0x00ffffff;

   while ( ( c = *( p++ ) ) != '.' )
   {
      id = ( id << 5 ) | ( id >> 19 );
      id += c;
      id &= mask;
   }
   if( id == 0 ) id = 1;

   return id;
}

static void find_paddemos(char *dir, EDumpOrCompile dumpOrCompile)
{
   WIN32_FIND_DATAA fd;
   HANDLE h;
   char flub[260];
   sprintf(flub, "%s\\*.*", dir);
   h = FindFirstFileA(flub, &fd);
   if (h != INVALID_HANDLE_VALUE)
   {
      do
      {
         sprintf(flub, "%s\\%s", dir, fd.cFileName);
         if (dumpOrCompile == kDump && string_ends_with(fd.cFileName, ".row"))
         {
            if (is_paddemo(flub))
            {
               char txtfile[260];
               strcpy(txtfile, flub);
               replace_extension(txtfile, "txt");

               if (!file_exists(txtfile) || compare_modified_times(flub, txtfile) > 0)
               {
                  printf("Dumping %s\n", flub);
                  dump_paddemo(flub);
               }
            }
         }
         else if (dumpOrCompile == kCompile && string_ends_with(fd.cFileName, ".txt"))
         {
            char rowfile[260];
            strcpy(rowfile, flub);
            replace_extension(rowfile, "row");

            if (file_exists(rowfile) && is_paddemo(rowfile) && compare_modified_times(flub, rowfile) > 0)
            {
               printf("Compiling %s\n", flub);
               parse_paddemo(flub);
            }
         }
         else if (dumpOrCompile == kFind && string_ends_with(fd.cFileName, ".row"))
         {
            char rowFile[260];
            strcpy(rowFile, fd.cFileName);
            if (GV_StrCode(rowFile) == gHashToFind)
            {
               printf("Matched %s\n", flub);
            }
         }
         else if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") && (GetFileAttributesA(flub) & FILE_ATTRIBUTE_DIRECTORY))
         {
            find_paddemos(flub, dumpOrCompile);
         }
      }
      while (FindNextFileA(h, &fd));
   }
}

int main(int argc, char *argv[])
{
   if (argc == 2)
   {
      if (string_ends_with(argv[1], ".row"))
      {
         dump_paddemo(argv[1]);
      }
      else if (string_ends_with(argv[1], ".txt"))
      {
         parse_paddemo(argv[1]);
      }
   }
   else if (argc == 3 && !strcmp(argv[1], "-d"))
   {
      find_paddemos(argv[2], kDump);
   }
   else if (argc == 3 && !strcmp(argv[1], "-c"))
   {
      find_paddemos(argv[2], kCompile);
   }
   else if (argc == 4 && !strcmp(argv[1], "-f"))
   {
      sscanf(argv[3], "%x", &gHashToFind);
      find_paddemos(argv[2], kFind);
   }
   else
   {
      printf("PadDemoTool\n");
      printf("Manipulates pad demo recordings\n");
      printf("Usage:\n");
      printf(" paddemotool w14a_pdemo_00.row\n");
      printf("  Dumps the row file to w14a_pdemo_00.txt.\n");
      printf(" paddemotool w14a_pdemo_00.txt\n");
      printf("  Compiles the text file to w14a_pdemo_00.row.\n  You will need to check it out first.\n");
      printf(" paddemotool -d c:\\cp4\\cp4a-dev\\mgs2\\gamedata\\assets\\row\n");
      printf("  Scans the given directory for paddemo files\n  and dumps them if newer.\n");
      printf(" paddemotool -c c:\\cp4\\cp4a-dev\\mgs2\\gamedata\\assets\\row\n");
      printf("  Scans the given directory for paddemo text files\n  and compiles them if newer.\n");
      printf(" paddemotool -f c:\\cp4\\cp4a-dev\\mgs2\\gamedata\\assets\\row f7f329\n");
      printf("  Scans the given directory for paddemo text files\n  and matches the paddemo with the given strcode.\n");
   }
   return 0;
}