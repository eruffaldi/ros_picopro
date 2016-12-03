#pragma once

#if 0

inline void convert_NV12_to_YUV422(const uint8_t * s, uint8_t * d, int w, int h)
{
	uint8_t * sy = s;
	uint8_t * suvi = sy + w*h;

	for(int i = 0; i < h; i++)
	{
		uint8_t * suv = suvi;

		//
		for(int j = 0; j < w; j+=2)
		{
			uint8_t uv[2] = { suv[0], suv[1]};
			
			// ++ u
			*dy++ = sy[0];
			// ++ v
			*dy++ = sy[1];
			sy += 2;
		}
	}
}


inline void convert_YUV422_to_NV12(const uint8_t * s, uint8_t * d, int w, int h)
{
	uint8_t * dy = d;
	uint8_t * duv = dy + w*h;
	for(int i = 0; i < h; i++)
	{
		for(int j = 0; j < w; j+=2)
		{
			// UYVY
			uint8_t uy[2] = { s[0], s[1]};
			uint8_t vy[2] = { s[2], s[3]};
			s += 4;

			*dy++ = uy[1];
			*dy++ = vy[1];

			// emit UV only for even lines
			if ((i & 1) == 0)
			{
				*duv++ = uy[0];
				*duv++ = vy[1];				
			}
		}
	}
}

#endif