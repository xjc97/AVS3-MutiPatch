#include "bs_process.h"
using namespace std;

static int xFindNextStartCode(FILE* fp, int* ruiPacketSize, unsigned char* pucBuffer)
{
    unsigned int uiDummy = 0;
    char bEndOfStream = 0;
    size_t ret = 0;
    ret = fread(&uiDummy, 1, 2, fp);
    if (ret != 2)
    {
        return -1;
    }

    if (feof(fp))
    {
        return -1;
    }
    assert(0 == uiDummy);

    ret = fread(&uiDummy, 1, 1, fp);
    if (ret != 1)
    {
        return -1;
    }
    if (feof(fp))
    {
        return -1;
    }
    assert(1 == uiDummy);

    int iNextStartCodeBytes = 0;
    unsigned int iBytesRead = 0;
    unsigned int uiZeros = 0;
    unsigned char pucBuffer_Temp[16];
    int iBytesRead_Temp = 0;

    pucBuffer[iBytesRead++] = 0x00;
    pucBuffer[iBytesRead++] = 0x00;
    pucBuffer[iBytesRead++] = 0x01;
    while (1)
    {
        unsigned char ucByte = 0;
        ret = fread(&ucByte, 1, 1, fp);

        if (feof(fp))
        {
            iNextStartCodeBytes = 0;
            bEndOfStream = 1;
            break;
        }
        pucBuffer[iBytesRead++] = ucByte;
        if (1 < ucByte)
        {
            uiZeros = 0;
        }
        else if (0 == ucByte)
        {
            uiZeros++;
        }
        else if (uiZeros > 1)
        {
            iBytesRead_Temp = 0;
            pucBuffer_Temp[iBytesRead_Temp] = ucByte;

            iBytesRead_Temp++;
            ret = fread(&ucByte, 1, 1, fp);
            if (ret != 1)
            {
                return -1;
            }
            pucBuffer_Temp[iBytesRead_Temp] = ucByte;
            pucBuffer[iBytesRead++] = ucByte;
            iBytesRead_Temp++;

            if (pucBuffer_Temp[0] == 0x01 && (pucBuffer_Temp[1] == 0xb3 || pucBuffer_Temp[1] == 0xb6 || pucBuffer_Temp[1] == 0xb0 || pucBuffer_Temp[1] == 0x00 || pucBuffer_Temp[1] == 0xb1))
            {
                iNextStartCodeBytes = 2 + 1 + 1;
                uiZeros = 0;
                break;
            }
            else
            {
                uiZeros = 0;
                iNextStartCodeBytes = 0;// 2 + 1 + 1;
            }
        }
        else
        {
            uiZeros = 0;
        }
    }
    *ruiPacketSize = iBytesRead - iNextStartCodeBytes;

    if (bEndOfStream)
    {
        return 0;
    }
    if (fseek(fp, -1 * iNextStartCodeBytes, SEEK_CUR) != 0)
    {
        printf("file seek failed!\n");
    };
    return 0;
}

unsigned int initParsingConvertPayloadToRBSP(const unsigned int uiBytesRead, unsigned char* pBuffer, unsigned char* pBuffer2)
{
    unsigned int uiZeroCount = 0;
    unsigned int uiBytesReadOffset = 0;
    unsigned int uiBitsReadOffset = 0;
    const unsigned char* pucRead = pBuffer;
    unsigned char* pucWrite = pBuffer2;
    unsigned int uiWriteOffset = uiBytesReadOffset;
    unsigned char ucCurByte = pucRead[uiBytesReadOffset];

    for (uiBytesReadOffset = 0; uiBytesReadOffset < uiBytesRead; uiBytesReadOffset++)
    {
        ucCurByte = pucRead[uiBytesReadOffset];
        if (2 <= uiZeroCount && 0x02 == pucRead[uiBytesReadOffset])
        {
            pucWrite[uiWriteOffset] = ((pucRead[uiBytesReadOffset] >> 2) << (uiBitsReadOffset + 2));
            uiBitsReadOffset += 2;
            uiZeroCount = 0;
            if (uiBitsReadOffset >= 8)
            {
                uiBitsReadOffset = 0;
                continue;
            }
            if (uiBytesReadOffset >= uiBytesRead)
            {
                break;
            }
        }
        else if (2 <= uiZeroCount && 0x01 == pucRead[uiBytesReadOffset])
        {
            uiBitsReadOffset = 0;
            pucWrite[uiWriteOffset] = pucRead[uiBytesReadOffset];
        }
        else
        {
            pucWrite[uiWriteOffset] = (pucRead[uiBytesReadOffset] << uiBitsReadOffset);
        }

        if (uiBytesReadOffset + 1 < uiBytesRead)
        {
            pucWrite[uiWriteOffset] |= (pucRead[uiBytesReadOffset + 1] >> (8 - uiBitsReadOffset));
        }
        uiWriteOffset++;

        if (0x00 == ucCurByte)
        {
            uiZeroCount++;
        }
        else
        {
            uiZeroCount = 0;
        }
    }

    // th just clear the remaining bits in the buffer
    for (unsigned int ui = uiWriteOffset; ui < uiBytesRead; ui++)
    {
        pucWrite[ui] = 0;
    }
    memcpy(pBuffer, pBuffer2, uiWriteOffset);
    pBuffer[uiWriteOffset] = 0x00;
    pBuffer[uiWriteOffset + 1] = 0x00;
    pBuffer[uiWriteOffset + 2] = 0x01;
    return uiBytesRead;
}

static int read_a_bs(FILE* fp, int* pos, unsigned char* bs_buf)
{
    int read_size, bs_size;
    unsigned char b = 0;
    bs_size = 0;
    read_size = 0;
    unsigned char* bs_buf2 = NULL;
    bs_buf2 = (unsigned char*)malloc(32 * 1024 * 1024);


    if (!fseek(fp, *pos, SEEK_SET))
    {
        int ret = 0;
        ret = xFindNextStartCode(fp, &bs_size, bs_buf);
        if (ret == -1)
        {
            return -1;
        }
        read_size = initParsingConvertPayloadToRBSP(bs_size, bs_buf, bs_buf2);
    }
    else
    {
        return -1;
    }
    free(bs_buf2);
    return read_size;
}

void bin_prosess_one_frame(
                           FILE* bs[PATCH_CNT], FILE* bin_3,
                           int bs_read_pos_patch[PATCH_CNT], int* bs_read_pos_3, int* bs_read_pos,
                           unsigned char* bs_tmp, unsigned char* bs_final, int frame_cnt
)
{
    int bs_size = 0;
    int patch_idx_pos[PATCH_CNT - 1] = { 0 };

    /** write the head info **/
    if (frame_cnt)
    {
        for (int i = 0; i < 2; i++)
        {
            bs_size = read_a_bs(bin_3, bs_read_pos_3, bs_tmp);
            if (i)
            {
                memcpy(bs_final + *bs_read_pos, bs_tmp, bs_size * sizeof(unsigned char));
                *bs_read_pos += bs_size;
            }
            *bs_read_pos_3 += bs_size;
        }
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            bs_size = read_a_bs(bin_3, bs_read_pos_3, bs_tmp);
            memcpy(bs_final + *bs_read_pos, bs_tmp, bs_size * sizeof(unsigned char));
            *bs_read_pos += bs_size;
            *bs_read_pos_3 += bs_size;
        }
    }

    for (int i = 0; i < PATCH_CNT; i++)
    {
        if (frame_cnt)
        {
            for (int j = 0; j < 2; j++)
            {
                bs_size = read_a_bs(bs[i], &bs_read_pos_patch[i], bs_tmp);
                if (j == 1)
                {
                    memcpy(bs_final + *bs_read_pos, bs_tmp, bs_size * sizeof(unsigned char));
                    *bs_read_pos += bs_size;
                }
                bs_read_pos_patch[i] += bs_size;
            }
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                bs_size = read_a_bs(bs[i], &bs_read_pos_patch[i], bs_tmp);
                if (j == 2)
                {
                    memcpy(bs_final + *bs_read_pos, bs_tmp, bs_size * sizeof(unsigned char));
                    *bs_read_pos += bs_size;
                }
                bs_read_pos_patch[i] += bs_size;
            }
        }
        if (i < PATCH_CNT - 1)
        {
            patch_idx_pos[i] = *bs_read_pos + 3;
        }
    }

    /** write patch 1 **/


    /** write patch 2 **/
    //if (frame_cnt)
    //{
    //    for (int i = 0; i < 2; i++)
    //    {
    //        bs_size = read_a_bs(bin_2, bs_read_pos_2, bs_tmp);
    //        if (i == 1)
    //        {
    //            memcpy(bs_final + *bs_read_pos, bs_tmp, bs_size * sizeof(unsigned char));
    //            *bs_read_pos += bs_size;
    //        }
    //        *bs_read_pos_2 += bs_size;
    //    }
    //}
    //else
    //{
    //    for (int i = 0; i < 3; i++)
    //    {
    //        bs_size = read_a_bs(bin_2, bs_read_pos_2, bs_tmp);
    //        if (i == 2)
    //        {
    //            memcpy(bs_final + *bs_read_pos, bs_tmp, bs_size * sizeof(unsigned char));
    //            *bs_read_pos += bs_size;
    //        }
    //        *bs_read_pos_2 += bs_size;
    //    }
    //}
    for (int i = 0; i < PATCH_CNT - 1; i++)
    {
        if (i==0)
        {
            bs_final[patch_idx_pos[i]] = 0x01;  //add patch idx 
        }
        else if (i == 1)
        {
            bs_final[patch_idx_pos[i]] = 0x02;  //add patch idx 
        }
        else
        {
            bs_final[patch_idx_pos[i]] = 0x03;  //add patch idx 
        }
    }
    
}

void bin_process(string bin[5], int frame_cnt)
{
    string file[PATCH_CNT];
    FILE* bs[PATCH_CNT];
    for (int i = 0; i < PATCH_CNT; i++)
    {
        file[i] = bin[i];
        file[i].append(".bin");
        bs[i] = fopen(file[i].data(), "rb");
    }

    FILE* bs_head = fopen("head.bin", "rb");
    if (is_file_exist("merge_bs.bin"))
    {
        remove("merge_bs.bin");
    }
    FILE* bs_out = fopen("merge_bs.bin", "ab");
    unsigned char* bs_final = NULL;
    unsigned char* bs_tmp = NULL;
    bs_final = (unsigned char*)malloc(32 * 1024 * 1024);
    bs_tmp = (unsigned char*)malloc(32 * 1024 * 1024);
    int bs_read_pos = 0;
    int bs_read_pos_patch[PATCH_CNT] = { 0 };

    int bs_read_pos_head = 0;

    for (int i = 0; i < frame_cnt; i++)
    {
        bin_prosess_one_frame(bs, bs_head, bs_read_pos_patch, &bs_read_pos_head, &bs_read_pos, bs_tmp, bs_final, i);
    }

    fwrite(bs_final, sizeof(unsigned char), bs_read_pos, bs_out);

    /** pic end flag **/

    unsigned char bit;
    bit = 0x00;
    fwrite(&bit, 1, 1, bs_out);
    bit = 0x00;
    fwrite(&bit, 1, 1, bs_out);
    bit = 0x01;
    fwrite(&bit, 1, 1, bs_out);
    bit = 0xb1;
    fwrite(&bit, 1, 1, bs_out);

    for (int i = 0; i < PATCH_CNT; i++)
    {
        fclose(bs[i]);
    }
    fclose(bs_head);
    fclose(bs_out);
    free(bs_final);
    free(bs_tmp);
}
