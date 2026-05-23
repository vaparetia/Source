
/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "../Framework3/Imaging/Image.h"
#include "../Framework3/Util/ConvexHull.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

struct WorkPacket
{
	ConvexHull Hull;
	float2 Polygon[8];
	float Area;
	float HullArea;
	uint Rotation;
};

uint FindOptimalRotation(float2 *vertices, uint vertex_count, uint *indices);

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		fprintf(stderr,
			"\nParticleTrimmer <imagefile> <threshold 0-255> <vertex_count 3-8> [options]\n\n"
			"\tOption | Type   | Default | Description\n"
			"\t-------+--------+---------+------------------------------------------------------------\n"
			"\t -ax   | int    |    1    | Atlas tile count in x\n"
			"\t -ay   | int    |    1    | Atlas tile count in y\n"
			"\t -sx   | float  |   1.0   | Scale factor of final x-coordinates\n"
			"\t -sy   | float  |   1.0   | Scale factor of final y-coordinates\n"
			"\t -bx   | float  |   0.0   | Bias factor of final x-coordinate\n"
			"\t -by   | float  |   0.0   | Bias factor of final y-coordinate\n"
			"\t -h    | int    |   50    | Maximum convex hull size to reduce to before optimization\n"
			"\t -sp   | int    |   16    | Sub-pixel division when generating the convex hull\n"
			"\t -d    | int    |    0    | Number of times to initially dilate the image\n"
			"\t -i    | int[n] |  NULL   | Index buffer. Used to optimize vertex ordering if provided.\n"
			"\t-------+--------+---------+------------------------------------------------------------\n"
		);

		return 0;
	}

	const char *file_name = argv[1];
	const int threshold = atoi(argv[2]);
	const float threshold_f = (float) threshold;
	const uint vertex_count = atoi(argv[3]);

	if (threshold > 255)
	{
		fprintf(stderr, "Error: Threshold %d out of range\n", threshold);
		return -1;
	}

	if (vertex_count < 3 || vertex_count > 8)
	{
		fprintf(stderr, "Error: Vertex count %d out of range\n", vertex_count);
		return -1;
	}

	float2 scale(1.0f, 1.0f);
	float2 bias(0.0f, 0.0f);
	uint atlas_x = 1;
	uint atlas_y = 1;
	uint max_hull_size = 50;
	int sub_pixel = 16;
	uint dilate_count = 0;

	uint indices[(8 - 2) * 3];
	bool use_index_buffer = false;


	// Parse optional commandline switches
	int arg = 4;
	while (arg < argc)
	{
		if (argv[arg][0] != '-')
			goto error;

		switch (argv[arg][1])
		{
		case 'a':
			if (argv[arg][2] == 'x')
				atlas_x = atoi(argv[++arg]);
			else if (argv[arg][2] == 'y')
				atlas_y = atoi(argv[++arg]);
			else
				goto error;
			break;
		case 'b':
			if (argv[arg][2] == 'x')
				bias.x = (float) atof(argv[++arg]);
			else if (argv[arg][2] == 'y')
				bias.y = (float) atof(argv[++arg]);
			else
				goto error;
			break;
		case 'd':
			if (argv[arg][2])
				goto error;
			dilate_count = atoi(argv[++arg]);
			break;
		case 'h':
			if (argv[arg][2])
				goto error;
			max_hull_size = atoi(argv[++arg]);
			break;
		case 'i':
			{
				if (argv[arg][2])
					goto error;

				const uint index_count = (vertex_count - 2) * 3;
				for (uint i = 0; i < index_count; i++)
				{
					++arg;
					if (arg >= argc)
					{
						fprintf(stderr, "Too short index buffer, %d indices needed\n", index_count);
						return -1;
					}
					if (argv[arg][0] < '0' || argv[arg][0] > '9')
					{
						fprintf(stderr, "Expected positive integer at \"%s\"\n", argv[arg]);
						return -1;
					}
					uint index = atoi(argv[arg]);
					if (index >= vertex_count)
					{
						fprintf(stderr, "Index \"%d\" out of range\n", index);
						return -1;
					}

					indices[i] = index;
				}

				use_index_buffer = true;
			}
			break;
		case 's':
			if (argv[arg][2] == 'x')
				scale.x = (float) atof(argv[++arg]);
			else if (argv[arg][2] == 'y')
				scale.y = (float) atof(argv[++arg]);
			else if (argv[arg][2] == 'p')
				sub_pixel = atoi(argv[++arg]);
			else
				goto error;
			break;
		default:
		error:
			fprintf(stderr, "Unknown option: \"%s\"\n", argv[arg]);
			return -1;
		}

		arg++;
	}


	const bool using_atlas = (atlas_x > 1 || atlas_y > 1);

	Image img;
	if (!img.loadImage(file_name))
	{
		fprintf(stderr, "Error: Couldn't load \"%s\"\n", file_name);
		return -1;
	}

	// Convert to single channel 8-bit
	img.uncompressImage();
	img.unpackImage();

	uint ch = getChannelCount(img.getFormat());
	if (ch == 4)
		img.removeChannels(false, false, false, true);
	else if (ch == 3)
		img.toGrayScale();
	else if (ch == 2)
		img.removeChannels(false, true);

	while (dilate_count--)
	{
		img.dilate();
	}

	const int w = img.getWidth();
	const int h = img.getHeight();
	ubyte *pixels = img.getPixels();

	// Threshold the pixels
/*
	int set_pixels = 0;
	for (int i = 0; i < w * h; i++)
	{
		uint c = (pixels[i] > threshold)? 1 : 0;
		//pixels[i] = c;
		set_pixels += c;
	}

	printf("Set pixels: %.2f%%\n", float(100 * set_pixels) / float(w * h));
*/
	const int tile_w = w / atlas_x;
	const int tile_h = h / atlas_y;


	scale.x *= 2.0f / tile_w;
	scale.y *= 2.0f / tile_h;


	// Set up convex hulls
	WorkPacket *packets = new WorkPacket[atlas_x * atlas_y];
	for (uint ay = 0; ay < atlas_y; ay++)
	{
		for (uint ax = 0; ax < atlas_x; ax++)
		{
			ConvexHull &hull = packets[ay * atlas_x + ax].Hull;

			const int start_x = ax * tile_w;
			const int start_y = ay * tile_h;
			const int end_x = start_x + tile_w - 1;
			const int end_y = start_y + tile_h - 1;

			const float off_x = 0.5f * (tile_w - 1);
			const float off_y = 0.5f * (tile_h - 1);
			const float corner_off_x = 0.5f * tile_w;
			const float corner_off_y = 0.5f * tile_h;

			// Corner cases
			if (pixels[start_y * w + start_x] > threshold)
			{
				hull.InsertPoint(float2(-corner_off_x, -corner_off_y));
			}

			if (pixels[start_y * w + end_x] > threshold)
			{
				hull.InsertPoint(float2(corner_off_x, -corner_off_y));
			}

			if (pixels[end_y * w + start_x] > threshold)
			{
				hull.InsertPoint(float2(-corner_off_x, corner_off_y));
			}

			if (pixels[end_y * w + end_x] > threshold)
			{
				hull.InsertPoint(float2(corner_off_x, corner_off_y));
			}

			// Edge cases
			ubyte *row = pixels + start_y * w;
			for (int x = start_x; x < end_x; x++)
			{
				int c0 = row[x + 0];
				int c1 = row[x + 1];

				if ((c0 > threshold) != (c1 > threshold))
				{
					float d0 = (float) c0;
					float d1 = (float) c1;

					float sub_pixel_x = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(float2(x - start_x - off_x + sub_pixel_x, -corner_off_y));
				}
			}

			row = pixels + end_y * w;
			for (int x = start_x; x < end_x; x++)
			{
				int c0 = row[x + 0];
				int c1 = row[x + 1];

				if ((c0 > threshold) != (c1 > threshold))
				{
					float d0 = (float) c0;
					float d1 = (float) c1;

					float sub_pixel_x = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(float2(x - start_x - off_x + sub_pixel_x, corner_off_y));
				}
			}

			ubyte *col = pixels + start_x;
			for (int y = start_y; y < end_y; y++)
			{
				int c0 = col[(y + 0) * w];
				int c1 = col[(y + 1) * w];

				if ((c0 > threshold) != (c1 > threshold))
				{
					float d0 = (float) c0;
					float d1 = (float) c1;

					float sub_pixel_y = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(float2(-corner_off_x, y - start_y - off_y + sub_pixel_y));
				}
			}

			col = pixels + end_x;
			for (int y = start_y; y < end_y; y++)
			{
				int c0 = col[(y + 0) * w];
				int c1 = col[(y + 1) * w];

				if ((c0 > threshold) != (c1 > threshold))
				{
					float d0 = (float) c0;
					float d1 = (float) c1;

					float sub_pixel_y = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(float2(corner_off_x, y - start_y - off_y + sub_pixel_y));
				}
			}




			// The interior pixels
			for (int y = start_y; y < end_y; y++)
			{
				ubyte *row0 = pixels + (y + 0) * w;
				ubyte *row1 = pixels + (y + 1) * w;

				for (int x = start_x; x < end_x; x++)
				{
					int c00 = row0[x + 0];
					int c01 = row0[x + 1];
					int c10 = row1[x + 0];
					int c11 = row1[x + 1];

					int count = 0;
					if (c00 > threshold) ++count;
					if (c01 > threshold) ++count;
					if (c10 > threshold) ++count;
					if (c11 > threshold) ++count;

					if (count > 0 && count < 4)
					{
						float d00 = (float) c00;
						float d01 = (float) c01;
						float d10 = (float) c10;
						float d11 = (float) c11;

						for (int n = 0; n <= sub_pixel; n++)
						{
							// Lerping factors
							float f0 = float(n) / float(sub_pixel);
							float f1 = 1.0f - f0;

							float x0 = d00 * f1 + d10 * f0;
							float x1 = d01 * f1 + d11 * f0;

							if ((x0 > threshold_f) != (x1 > threshold_f))
							{
								float sub_pixel_x = (threshold_f - x0) / (x1 - x0);
								hull.InsertPoint(float2(x - start_x - off_x + sub_pixel_x, y - start_y - off_y + f0));
							}

							float y0 = d00 * f1 + d01 * f0;
							float y1 = d10 * f1 + d11 * f0;

							if ((y0 > threshold_f) != (y1 > threshold_f))
							{
								float sub_pixel_y = (threshold_f - y0) / (y1 - y0);
								hull.InsertPoint(float2(x - start_x - off_x + f0, y - start_y - off_y + sub_pixel_y));
							}
						}
					}

				}

			}

			fprintf(stderr, "Convex hull has %d vertices\n", hull.GetCount());
			if (hull.GetCount() > max_hull_size)
			{
				float area_before = hull.GetArea();

				do
				{
					if (!hull.RemoveLeastRelevantEdge())
					{
						break;
					}
				} while (hull.GetCount() > max_hull_size);

				float area_after = hull.GetArea();

				fprintf(stderr, "Convex hull was reduced to %d vertices. Area expanded from %.2f%% to %.2f%%\n", hull.GetCount(), 100.0f * area_before / (tile_w * tile_h), 100.0f * area_after / (tile_w * tile_h));
			}
		}
	}


	// Do the heavy work
	for (uint n = 0; n < atlas_x * atlas_y; n++)
	{
		WorkPacket &packet = packets[n];

		const uint count = packet.Hull.FindOptimalPolygon(packet.Polygon, vertex_count, &packet.Area);
		packet.HullArea = packet.Hull.GetArea();

		// Scale-bias the results
		for (uint i = 0; i < count; i++)
		{
			packet.Polygon[i] = packet.Polygon[i] * scale + bias;
		}

		// If fewer vertices were returned than asked for, just repeat the last vertex
		for (uint i = count; i < vertex_count; i++)
		{
			packet.Polygon[i] = packet.Polygon[count - 1];
		}

		// Optimize vertex ordering
		if (use_index_buffer)
			packet.Rotation = FindOptimalRotation(packet.Polygon, vertex_count, indices);
		else
			packet.Rotation = 0;
	}


	// Output the results
	if (using_atlas)
	{
		printf("\nfloat g_ParticleCoords[%d][%d][2] =\n{\n", atlas_x * atlas_y, vertex_count);
	}
	else
	{
		printf("\nfloat g_ParticleCoords[%d][2] =\n{\n", vertex_count);
	}
	for (uint ay = 0; ay < atlas_y; ay++)
	{
		for (uint ax = 0; ax < atlas_x; ax++)
		{
			WorkPacket &packet = packets[ay * atlas_x + ax];
			float2 *polygon = packet.Polygon;

			printf("\t// Area reduced to %.2f%% (optimal convex area is %.2f%%)\n", 100 * (packet.Area / (tile_w * tile_h)), 100 * (packet.HullArea / (tile_w * tile_h)));
			if (using_atlas)
			{
				printf("\t{\n");
				for (uint i = 0; i < vertex_count; i++)
				{
					uint index = (i + packet.Rotation) % vertex_count;
					printf("\t\t{ % ff, % ff },\n", polygon[index].x, polygon[index].y);
				}
				printf("\t},\n\n");
			}
			else
			{
				for (uint i = 0; i < vertex_count; i++)
				{
					uint index = (i + packet.Rotation) % vertex_count;
					printf("\t{ % ff, % ff },\n", polygon[index].x, polygon[index].y);
				}
			}
		}
	}

	printf("};\n");

	delete [] packets;

	return 0;
}

uint FindOptimalRotation(float2 *vertices, uint vertex_count, uint *indices)
{
	const uint index_count = (vertex_count - 2) * 3;

	uint optimal = 0;
	float min_length = FLT_MAX;

	for (uint i = 0; i < vertex_count; i++)
	{
		float sum = 0;
		for (uint k = 0; k < index_count; k += 3)
		{
			uint i0 = (indices[k + 0] + i) % vertex_count;
			uint i1 = (indices[k + 1] + i) % vertex_count;
			uint i2 = (indices[k + 2] + i) % vertex_count;

			const float2 &v0 = vertices[i0];
			const float2 &v1 = vertices[i1];
			const float2 &v2 = vertices[i2];

			sum += distance(v0, v1);
			sum += distance(v1, v2);
			sum += distance(v2, v0);
		}

		if (sum < min_length)
		{
			optimal = i;
			min_length = sum;
		}
	}

	return optimal;
}
