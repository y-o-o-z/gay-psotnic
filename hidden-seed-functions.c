void gen_cfg_seed(unsigned char *out)
{
    int i;
    out[0] = 202;
    out[1] = 32;
    out[2] = 238;
    out[3] = 133;
    out[4] = 219;
    out[5] = 53;
    out[6] = 121;
    out[7] = 145;
    out[8] = 18;
    out[9] = 32;
    out[10] = 227;
    out[11] = 53;
    out[12] = 55;
    out[13] = 237;
    out[14] = 148;
    out[15] = 58;

    for(i=0; i<16; ++i)
    {
        out[i] ^= 232;
    }
}

void gen_ul_seed(unsigned char *out)
{
    int i;
    out[0] = 119;
    out[1] = 101;
    out[2] = 13;
    out[3] = 182;
    out[4] = 215;
    out[5] = 102;
    out[6] = 187;
    out[7] = 25;
    out[8] = 184;
    out[9] = 60;
    out[10] = 187;
    out[11] = 181;
    out[12] = 207;
    out[13] = 82;
    out[14] = 125;
    out[15] = 3;

    for(i=0; i<16; ++i)
    {
        out[i] ^= 44;
    }
}

