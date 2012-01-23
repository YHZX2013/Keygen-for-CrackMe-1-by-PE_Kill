/* MD4 Collision Generator by Patrick Stach <pstach@stachliu.com>
 * Implementation of paper by Xiaoyun Wang, et. al.
 *
 * A few optimizations to make the solving method a bit more deterministic
 *
 * Usage:
 * 	./md4coll or ./md4coll IV0 IV1 IV2 IV3
 *
 * Requires being built 32 bit (unsigned int as 32 bit)
 *
 *  06-Feb-2006: modified for Windows compatibility by Hendrik Reh <hendrik.reh@o5h.de> along the lines of Steve Dispensa
 *              
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _MSC_VER
#include <windows.h>
#include <wincrypt.h>
#include <strsafe.h>
#define random  Random//winRandom
#else
#include <unistd.h>
#endif

#include "md4coll.h"

#define IV0	0x67452301
#define IV1	0xefcdab89
#define IV2	0x98badcfe
#define IV3	0x10325476

#define RL(x, y) (((x) << (y)) | ((x) >> (32 - (y))))
#define RR(x, y) (((x) >> (y)) | ((x) << (32 - (y))))

#define F(b, c, d) (((c ^ d) & b) ^ d)
#define G(b, c, d) ((b & c) | (b & d) | (c & d))
#define H(b, c, d) (b ^ c ^ d)

#define K1	0x5A827999
#define K2	0x6ED9EBA1

unsigned int IV[4] = { IV0, IV1, IV2, IV3 };

unsigned int Q0[49], Q1[49];

HCRYPTPROV cryptHandle;

static unsigned int randseed = 0;

int initRandom(void)
{
#ifdef _MSC_VER
	/*
	if(!CryptAcquireContext(&cryptHandle, NULL, NULL, PROV_RSA_FULL, 0 ) &&
	   !CryptAcquireContext(&cryptHandle, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET ))
	{
		char buf[100];
		StringCbPrintf((STRSAFE_LPWSTR)buf, sizeof(buf), (STRSAFE_LPWSTR)"error getting crypto context: 0x%x", GetLastError());
		MessageBox(0, (LPCWSTR)buf, (LPCWSTR)"error", MB_ICONERROR|MB_OK);
		return 0;
	}
	*/
	randseed = GetTickCount();
#else
	srandom(time(NULL) ^ (getpid() << 16));
#endif
	return 1;
}

long winRandom(void)
{
	int ret = 0;

	if(!CryptGenRandom(cryptHandle, sizeof(int), (BYTE*)&ret))
	{
		char buf[100];
		StringCbPrintf((STRSAFE_LPSTR)buf, sizeof(buf), (STRSAFE_LPSTR)"error generating random number: 0x%x", GetLastError());
		MessageBox(0, (LPCSTR)buf, (LPCSTR)"error", MB_ICONERROR|MB_OK);
		return 0;
	}

	return ret;
}

unsigned int Random()
{
	unsigned int result = randseed * 0x08088405;

	randseed = result + 1;

	return result;
}

void md4gen(unsigned char b, unsigned char c)
{
	size_t i;

md4_again:
	for(;;)
	{
		/* D1 */
		Q0[ 2] = random() & ~0x00000040;
		Q1[ 2] = Q0[ 2] - 0xffffffc0;

		/* C1 */
		Q0[ 3] = (random() | 0x000000c0) & ~(0x00000800 | 0x02000000);
		Q0[ 3] |= (Q0[ 2] & 0x02000000);
		Q1[ 3] = Q0[ 3] - 0xfffffc80;

		/* B1 */
		Q0[ 4] = (random() | 0x00000040) & ~0x02000480;
		Q1[ 4] = Q0[ 4] - 0xfe000000;

		/* A2 */
		Q0[ 5] = (random() | 0x00000480) & ~(0x02000000 | 0x00002000);
		Q0[ 5] |= (Q0[ 4] & 0x00002000);
		Q1[ 5] = Q0[ 5];

		/* D2 */
		Q0[ 6] = (random() | 0x02000000) & ~(0x00002000 | 0x003c0000);
		Q0[ 6] |= (Q0[ 5] & 0x003c0000);
		Q1[ 6] = Q0[ 6] - 0xffffe000;

		X0[ 5] = RR(Q0[ 6],  7) - F(Q0[ 5], Q0[ 4], Q0[ 3]) - Q0[ 2];
		X1[ 5] = RR(Q1[ 6],  7) - F(Q1[ 5], Q1[ 4], Q1[ 3]) - Q1[ 2];
		if(X0[ 5] != X1[ 5])
			continue;
		else
			if (((X0[5] >> c) & 0xFF) != b)
				continue;

		/* C2 */
		Q0[ 7] = (random() | 0x00100000) & ~(0x002c2000 | 0x00005000);
		Q0[ 7] |= (Q0[ 6] & 0x00005000);
		Q1[ 7] = Q0[ 7] - 0xffe40000;

		X0[ 6] = RR(Q0[ 7], 11) - F(Q0[ 6], Q0[ 5], Q0[ 4]) - Q0[ 3];
		X1[ 6] = RR(Q1[ 7], 11) - F(Q1[ 6], Q1[ 5], Q1[ 4]) - Q1[ 3];
		if(X0[ 6] != X1[ 6])
			continue;

		/* B2 */
		Q0[ 8] = (random() | 0x00003000) & ~(0x003c4000 | 0x00010000);
		Q0[ 8] |= (Q0[ 7] & 0x00010000);
		Q1[ 8] = Q0[ 8] - 0xfffff000;

		X0[ 7] = RR(Q0[ 8], 19) - F(Q0[ 7], Q0[ 6], Q0[ 5]) - Q0[ 4];
		X1[ 7] = RR(Q1[ 8], 19) - F(Q1[ 7], Q1[ 6], Q1[ 5]) - Q1[ 4];
		if(X0[ 7] != X1[ 7])
			continue;

		/* A3 */
		Q0[ 9] = (random() | 0x00207000) & ~(0x001d0000 | 0x02400000);
		Q0[ 9] |= (Q0[ 8] & 0x02400000);
		Q1[ 9] = Q0[ 9] - 0xffff0000;

		X0[ 8] = RR(Q0[ 9],  3) - F(Q0[ 8], Q0[ 7], Q0[ 6]) - Q0[ 5];
		X1[ 8] = RR(Q1[ 9],  3) - F(Q1[ 8], Q1[ 7], Q1[ 6]) - Q1[ 5];
		if(X0[ 8] != X1[ 8])
			continue;

		/* D3 */
		Q0[10] = (random() | 0x02307000) & ~(0x00490000 | 0x20000000);
		Q0[10] |= (Q0[ 9] & 0x20000000);
		Q1[10] = Q0[10] - 0x01e80000;

		X0[ 9] = RR(Q0[10],  7) - F(Q0[ 9], Q0[ 8], Q0[ 7]) - Q0[ 6];
		X1[ 9] = RR(Q1[10],  7) - F(Q1[ 9], Q1[ 8], Q1[ 7]) - Q1[ 6];
		if(X0[ 9] != X1[ 9])
			continue;

		/* C3 */
		Q0[11] = (random() | 0x20010000) & ~(0x02780000 | 0x80000000);
		Q0[11] |= (Q0[10] & 0x80000000);
		Q1[11] = Q0[11] - 0x20000000;

		X0[10] = RR(Q0[11], 11) - F(Q0[10], Q0[ 9], Q0[ 8]) - Q0[ 7];
		X1[10] = RR(Q1[11], 11) - F(Q1[10], Q1[ 9], Q1[ 8]) - Q1[ 7];
		if(X0[10] != X1[10])
			continue;

		/* B3 */
		Q0[12] = (random() | 0x02300000) & ~(0xa0080000 | 0x00400000);
		Q0[12] |= (Q0[11] & 0x00400000);
		Q1[12] = Q0[12] - 0x80000000;

		X0[11] = RR(Q0[12], 19) - F(Q0[11], Q0[10], Q0[ 9]) - Q0[ 8];
		X1[11] = RR(Q1[12], 19) - F(Q1[11], Q1[10], Q1[ 9]) - Q1[ 8];
		if(X0[11] != X1[11])
			continue;

		/* A4 */
		Q0[13] = (random() | 0x20000000) & ~(0x82400000 | 0x14000000);
		Q0[13] |= (Q0[12] & 0x14000000);
		Q1[13] = Q0[13] - 0xfdc00000;

		X0[12] = RR(Q0[13],  3) - F(Q0[12], Q0[11], Q0[10]) - Q0[ 9];
		X1[12] = RR(Q1[13],  3) - F(Q1[12], Q1[11], Q1[10]) - Q1[ 9];
		if((X0[12] ^ X1[12]) != 0x00010000)
			continue;

		/* D4 */
		Q0[14] = (random() | 0x94000000) & ~0x22400000;
		Q1[14] = Q0[14] - 0xf4000000;

		X0[13] = RR(Q0[14],  7) - F(Q0[13], Q0[12], Q0[11]) - Q0[10];
		X1[13] = RR(Q1[14],  7) - F(Q1[13], Q1[12], Q1[11]) - Q1[10];
		if(X0[13] != X1[13])
			continue;

		/* C4 */
		Q0[15] = (random() | 0x02400000) & ~(0x34000000 | 0x00040000);
		Q0[15] |= (Q0[14] & 0x00040000);
		Q1[15] = Q0[15];

		X0[14] = RR(Q0[15], 11) - F(Q0[14], Q0[13], Q0[12]) - Q0[11];
		X1[14] = RR(Q1[15], 11) - F(Q1[14], Q1[13], Q1[12]) - Q1[11];
		if(X0[14] != X1[14])
			continue;

		/* B4 */
		Q0[16] = (random() | 0x14000000) & ~0x20040000;
		Q1[16] = Q0[16] - 0xfffc0000;

		X0[15] = RR(Q0[16], 19) - F(Q0[15], Q0[14], Q0[13]) - Q0[12];
		X1[15] = RR(Q1[16], 19) - F(Q1[15], Q1[14], Q1[13]) - Q1[12];
		if(X0[15] != X1[15])
			continue;

		break;
	}

#define LOOP_11 100

	for(i = 0; i < LOOP_11; i++)
	{
		Q0[ 1] = random() & ~(0x00000040 | 0x00000480);
		Q0[ 1] |= (IV[1] & 0x00000040) | (Q0[ 2] & 0x00000480);
		Q1[ 1] = Q0[ 1];

		X0[ 0] = RR(Q0[ 1],  3) - F(IV[1], IV[2], IV[3]) - IV[0];
		/*
		if ((X0[0] & 0xFF) > 0xAC)
			continue;
		*/
		X1[ 0] = X0[ 0];

		
		X0[ 1] = RR(Q0[ 2],  7) - F(Q0[ 1], IV[1], IV[2]) - IV[3];
		X1[ 1] = RR(Q1[ 2],  7) - F(Q1[ 1], IV[1], IV[2]) - IV[3];
		if((X0[ 1] ^ X1[ 1]) != 0x80000000)
			continue;

		X0[ 2] = RR(Q0[ 3], 11) - F(Q0[ 2], Q0[ 1], IV[1]) - IV[2];
		X1[ 2] = RR(Q1[ 3], 11) - F(Q1[ 2], Q1[ 1], IV[1]) - IV[2];
		if((X0[ 2] ^ X1[ 2]) != 0x90000000)
			continue;

		X0[ 3] = RR(Q0[ 4], 19) - F(Q0[ 3], Q0[ 2], Q0[ 1]) - IV[1];
		X1[ 3] = RR(Q1[ 4], 19) - F(Q1[ 3], Q1[ 2], Q1[ 1]) - IV[1];
		if(X0[ 3] != X1[ 3])
			continue;

		X0[ 4] = RR(Q0[ 5],  3) - F(Q0[ 4], Q0[ 3], Q0[ 2]) - Q0[ 1];
		X1[ 4] = RR(Q1[ 5],  3) - F(Q1[ 4], Q1[ 3], Q1[ 2]) - Q1[ 1];
		if(X0[ 4] != X1[ 4])
			continue;

		Q0[17] = RL(G(Q0[16], Q0[15], Q0[14]) + Q0[13]
			+ X0[ 0] + K1,  3);
		Q1[17] = RL(G(Q1[16], Q1[15], Q1[14]) + Q1[13]
			+ X1[ 0] + K1,  3);
		if((Q0[17] - Q1[17]) != 0x8e000000)
			continue;

		Q0[18] = RL(G(Q0[17], Q0[16], Q0[15]) + Q0[14]
			+ X0[ 4] + K1,  5);
		Q1[18] = RL(G(Q1[17], Q1[16], Q1[15]) + Q1[14]
			+ X1[ 4] + K1,  5);
		if(Q0[18] != Q1[18])
			continue;

                Q0[19] = RL(G(Q0[18], Q0[17], Q0[16]) + Q0[15]
                        + X0[ 8] + K1,  9);
                Q1[19] = RL(G(Q1[18], Q1[17], Q1[16]) + Q1[15]
                        + X1[ 8] + K1,  9);
		if(Q0[19] != Q1[19])
			continue;

                Q0[20] = RL(G(Q0[19], Q0[18], Q0[17]) + Q0[16]
                        + X0[12] + K1, 13);
                Q1[20] = RL(G(Q1[19], Q1[18], Q1[17]) + Q1[16]
                        + X1[12] + K1, 13);
		if((Q0[20] - Q1[20]) != 0xa0000000)
			continue;
		break;
	}
	if(i >= LOOP_11)
		goto md4_again;

#define LOOP_12	0x4000000

        for(i = 0; i < LOOP_12; i++)
        {
                Q0[ 1] = random() & ~(0x00000040 | 0x00000480);
                Q0[ 1] |= (IV[1] & 0x00000040) | (Q0[ 2] & 0x00000480);
                Q1[ 1] = Q0[ 1];

                X0[ 0] = RR(Q0[ 1],  3) - F(IV[1], IV[2], IV[3]) - IV[0];
                X1[ 0] = X0[ 0];


                X0[ 1] = RR(Q0[ 2],  7) - F(Q0[ 1], IV[1], IV[2]) - IV[3];
                X1[ 1] = RR(Q1[ 2],  7) - F(Q1[ 1], IV[1], IV[2]) - IV[3];
                if((X0[ 1] ^ X1[ 1]) != 0x80000000)
                        continue;

                X0[ 2] = RR(Q0[ 3], 11) - F(Q0[ 2], Q0[ 1], IV[1]) - IV[2];
                X1[ 2] = RR(Q1[ 3], 11) - F(Q1[ 2], Q1[ 1], IV[1]) - IV[2];
                if((X0[ 2] ^ X1[ 2]) != 0x90000000)
                        continue;

                X0[ 3] = RR(Q0[ 4], 19) - F(Q0[ 3], Q0[ 2], Q0[ 1]) - IV[1];
                X1[ 3] = RR(Q1[ 4], 19) - F(Q1[ 3], Q1[ 2], Q1[ 1]) - IV[1];
                if(X0[ 3] != X1[ 3])
                        continue;

                X0[ 4] = RR(Q0[ 5],  3) - F(Q0[ 4], Q0[ 3], Q0[ 2]) - Q0[ 1];
                X1[ 4] = RR(Q1[ 5],  3) - F(Q1[ 4], Q1[ 3], Q1[ 2]) - Q1[ 1];
                if(X0[ 4] != X1[ 4])
                        continue;

                Q0[17] = RL(G(Q0[16], Q0[15], Q0[14]) + Q0[13]
                        + X0[ 0] + K1,  3);
                Q1[17] = RL(G(Q1[16], Q1[15], Q1[14]) + Q1[13]
                        + X1[ 0] + K1,  3);
                if((Q0[17] - Q1[17]) != 0x8e000000)
                        continue;

                Q0[18] = RL(G(Q0[17], Q0[16], Q0[15]) + Q0[14]
                        + X0[ 4] + K1,  5);
                Q1[18] = RL(G(Q1[17], Q1[16], Q1[15]) + Q1[14]
                        + X1[ 4] + K1,  5);
                if(Q0[18] != Q1[18])
                        continue;

                Q0[19] = RL(G(Q0[18], Q0[17], Q0[16]) + Q0[15]
                        + X0[ 8] + K1,  9);
                Q1[19] = RL(G(Q1[18], Q1[17], Q1[16]) + Q1[15]
                        + X1[ 8] + K1,  9);
		if(Q0[19] != Q1[19])
			continue;

                Q0[20] = RL(G(Q0[19], Q0[18], Q0[17]) + Q0[16]
                        + X0[12] + K1, 13);
                Q1[20] = RL(G(Q1[19], Q1[18], Q1[17]) + Q1[16]
                        + X1[12] + K1, 13);
		if((Q0[20] - Q1[20]) != 0xa0000000)
			continue;

                Q0[21] = RL(G(Q0[20], Q0[19], Q0[18]) + Q0[17]
                        + X0[ 1] + K1,  3);
                Q1[21] = RL(G(Q1[20], Q1[19], Q1[18]) + Q1[17]
                        + X1[ 1] + K1,  3);
		if((Q0[21] - Q1[21]) != 0x70000000)
			continue;

                Q0[22] = RL(G(Q0[21], Q0[20], Q0[19]) + Q0[18]
                        + X0[ 5] + K1,  5);
                Q1[22] = RL(G(Q1[21], Q1[20], Q1[19]) + Q1[18]
                        + X1[ 5] + K1,  5);
		if(Q0[22] != Q1[22])
			continue;

                Q0[23] = RL(G(Q0[22], Q0[21], Q0[20]) + Q0[19]
                        + X0[ 9] + K1,  9);
                Q1[23] = RL(G(Q1[22], Q1[21], Q1[20]) + Q1[19]
                        + X1[ 9] + K1,  9);
		if(Q0[23] != Q1[23])
			continue;

                Q0[24] = RL(G(Q0[23], Q0[22], Q0[21]) + Q0[20]
                        + X0[13] + K1, 13);
                Q1[24] = RL(G(Q1[23], Q1[22], Q1[21]) + Q1[20]
                        + X1[13] + K1, 13);
		if(Q0[24] != Q1[24])
			continue;

                Q0[25] = RL(G(Q0[24], Q0[23], Q0[22]) + Q0[21]
                        + X0[ 2] + K1,  3);
                Q1[25] = RL(G(Q1[24], Q1[23], Q1[22]) + Q1[21]
                        + X1[ 2] + K1,  3);
		if(Q0[25] != Q1[25])
			continue;

                Q0[26] = RL(G(Q0[25], Q0[24], Q0[23]) + Q0[22]
                        + X0[ 6] + K1,  5);
                Q1[26] = RL(G(Q1[25], Q1[24], Q1[23]) + Q1[22]
                        + X1[ 6] + K1,  5);
		if(Q0[26] != Q1[26])
			continue;

                Q0[27] = RL(G(Q0[26], Q0[25], Q0[24]) + Q0[23]
                        + X0[10] + K1,  9);
                Q1[27] = RL(G(Q1[26], Q1[25], Q1[24]) + Q1[23]
                        + X1[10] + K1,  9);
		if(Q0[27] != Q1[27])
			continue;

                Q0[28] = RL(G(Q0[27], Q0[26], Q0[25]) + Q0[24]
                        + X0[14] + K1, 13);
                Q1[28] = RL(G(Q1[27], Q1[26], Q1[25]) + Q1[24]
                        + X1[14] + K1, 13);
		if(Q0[28] != Q1[28])
			continue;

                Q0[29] = RL(G(Q0[28], Q0[27], Q0[26]) + Q0[25]
                        + X0[ 3] + K1,  3);
                Q1[29] = RL(G(Q1[28], Q1[27], Q1[26]) + Q1[25]
                        + X1[ 3] + K1,  3);
		if(Q0[29] != Q1[29])
			continue;

                Q0[30] = RL(G(Q0[29], Q0[28], Q0[27]) + Q0[26]
                        + X0[ 7] + K1,  5);
                Q1[30] = RL(G(Q1[29], Q1[28], Q1[27]) + Q1[26]
                        + X1[ 7] + K1,  5);
		if(Q0[30] != Q1[30])
			continue;

                Q0[31] = RL(G(Q0[30], Q0[29], Q0[28]) + Q0[27]
                        + X0[11] + K1,  9);
                Q1[31] = RL(G(Q1[30], Q1[29], Q1[28]) + Q1[27]
                        + X1[11] + K1,  9);
		if(Q0[31] != Q1[31])
			continue;

                Q0[32] = RL(G(Q0[31], Q0[30], Q0[29]) + Q0[28]
                        + X0[15] + K1, 13);
                Q1[32] = RL(G(Q1[31], Q1[30], Q1[29]) + Q1[28]
                        + X1[15] + K1, 13);
		if(Q0[32] != Q1[32])
			continue;

                Q0[33] = RL(H(Q0[32], Q0[31], Q0[30]) + Q0[29]
                        + X0[ 0] + K2,  3);
                Q1[33] = RL(H(Q1[32], Q1[31], Q1[30]) + Q1[29]
                        + X1[ 0] + K2,  3);
		if(Q0[33] != Q1[33])
			continue;

                Q0[34] = RL(H(Q0[33], Q0[32], Q0[31]) + Q0[30]
                        + X0[ 8] + K2,  9);
                Q1[34] = RL(H(Q1[33], Q1[32], Q1[31]) + Q1[30]
                        + X1[ 8] + K2,  9);
		if(Q0[34] != Q1[34])
			continue;

                Q0[35] = RL(H(Q0[34], Q0[33], Q0[32]) + Q0[31]
                        + X0[ 4] + K2, 11);
                Q1[35] = RL(H(Q1[34], Q1[33], Q1[32]) + Q1[31]
                        + X1[ 4] + K2, 11);
		if(Q0[35] != Q1[35])
			continue;

                Q0[36] = RL(H(Q0[35], Q0[34], Q0[33]) + Q0[32]
                        + X0[12] + K2, 15);
                Q1[36] = RL(H(Q1[35], Q1[34], Q1[33]) + Q1[32]
                        + X1[12] + K2, 15);
		if((Q0[36] ^ Q1[36]) != 0x80000000)
			continue;

                Q0[37] = RL(H(Q0[36], Q0[35], Q0[34]) + Q0[33]
                        + X0[ 2] + K2,  3);
                Q1[37] = RL(H(Q1[36], Q1[35], Q1[34]) + Q1[33]
                        + X1[ 2] + K2,  3);
		if((Q0[37] ^ Q1[37]) != 0x80000000)
			continue;

                Q0[38] = RL(H(Q0[37], Q0[36], Q0[35]) + Q0[34]
                        + X0[10] + K2,  9);
                Q1[38] = RL(H(Q1[37], Q1[36], Q1[35]) + Q1[34]
                        + X1[10] + K2,  9);
		if(Q0[38] != Q1[38])
			continue;

                Q0[39] = RL(H(Q0[38], Q0[37], Q0[36]) + Q0[35]
                        + X0[ 6] + K2, 11);
                Q1[39] = RL(H(Q1[38], Q1[37], Q1[36]) + Q1[35]
                        + X1[ 6] + K2, 11);
		if(Q0[39] != Q1[39])
			continue;

                Q0[40] = RL(H(Q0[39], Q0[38], Q0[37]) + Q0[36]
                        + X0[14] + K2, 15);
                Q1[40] = RL(H(Q1[39], Q1[38], Q1[37]) + Q1[36]
                        + X1[14] + K2, 15);
		if(Q0[40] != Q1[40])
			continue;

                Q0[41] = RL(H(Q0[40], Q0[39], Q0[38]) + Q0[37]
                        + X0[ 1] + K2,  3);
                Q1[41] = RL(H(Q1[40], Q1[39], Q1[38]) + Q1[37]
                        + X1[ 1] + K2,  3);
		if(Q0[41] != Q1[41])
			continue;

                Q0[42] = RL(H(Q0[41], Q0[40], Q0[39]) + Q0[38]
                        + X0[ 9] + K2,  9);
                Q1[42] = RL(H(Q1[41], Q1[40], Q1[39]) + Q1[38]
                        + X1[ 9] + K2,  9);
		if(Q0[42] != Q1[42])
			continue;

                Q0[43] = RL(H(Q0[42], Q0[41], Q0[40]) + Q0[39]
                        + X0[ 5] + K2, 11);
                Q1[43] = RL(H(Q1[42], Q1[41], Q1[40]) + Q1[39]
                        + X1[ 5] + K2, 11);
		if(Q0[43] != Q1[43])
			continue;

                Q0[44] = RL(H(Q0[43], Q0[42], Q0[41]) + Q0[40]
                        + X0[13] + K2, 15);
                Q1[44] = RL(H(Q1[43], Q1[42], Q1[41]) + Q1[40]
                        + X1[13] + K2, 15);
		if(Q0[44] != Q1[44])
			continue;

                Q0[45] = RL(H(Q0[44], Q0[43], Q0[42]) + Q0[41]
                        + X0[ 3] + K2,  3);
                Q1[45] = RL(H(Q1[44], Q1[43], Q1[42]) + Q1[41]
                        + X1[ 3] + K2,  3);
		if(Q0[45] != Q1[45])
			continue;

                Q0[46] = RL(H(Q0[45], Q0[44], Q0[43]) + Q0[42]
                        + X0[11] + K2,  9);
                Q1[46] = RL(H(Q1[45], Q1[44], Q1[43]) + Q1[42]
                        + X1[11] + K2,  9);
		if(Q0[46] != Q1[46])
			continue;

                Q0[47] = RL(H(Q0[46], Q0[45], Q0[44]) + Q0[43]
                        + X0[ 7] + K2, 11);
                Q1[47] = RL(H(Q1[46], Q1[45], Q1[44]) + Q1[43]
                        + X1[ 7] + K2, 11);
		if(Q0[47] != Q1[47])
			continue;

                Q0[48] = RL(H(Q0[47], Q0[46], Q0[45]) + Q0[44]
                        + X0[15] + K2, 15);
                Q1[48] = RL(H(Q1[47], Q1[46], Q1[45]) + Q1[44]
                        + X1[15] + K2, 15);
		if(Q0[48] != Q1[48])
			continue;
                break;
        }
	if(i >= LOOP_12)
		goto md4_again;
	return;
}