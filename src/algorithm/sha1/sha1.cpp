#include "../../inc/basic.h" 

__NS_BASIC_START

#define SHA1_MAC_LEN 20

typedef struct{
    DWORD state[5];
    DWORD count[2];
    BYTE buffer[64];
} SHA1_CTX;

void SHA1Reset(SHA1_CTX *context);
void SHA1Input(SHA1_CTX *context, BYTE *data, DWORD len);
void SHA1Result(SHA1_CTX *context, BYTE *digest);//20   
void SHA1Transform(DWORD *state, BYTE *buffer); //5  64   

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))  

                                                /* blk0() and blk() perform the initial expand. */
                                                /* I got the idea of expanding during the round function from SSLeay */

#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | (rol(block->l[i], 8) & 0x00FF00FF))   
#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))  

                                                /* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5);w = rol(w, 30);   
#define R1(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5);w = rol(w, 30);   
#define R2(v,w,x,y,z,i) z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);   
#define R3(v,w,x,y,z,i) z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5);w = rol(w, 30);   
#define R4(v,w,x,y,z,i) z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5);w=rol(w, 30);   

                                                /* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1Transform(DWORD *state, BYTE *buffer){
    DWORD a, b, c, d, e;
    typedef union{
        BYTE c[64];
        DWORD l[16];
    } CHAR64LONG16;
    CHAR64LONG16 *block;

    DWORD workspace[16];
    block = (CHAR64LONG16 *)workspace;
    memcpy(block, buffer, 64);

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a, b, c, d, e, 0); R0(e, a, b, c, d, 1); R0(d, e, a, b, c, 2); R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4); R0(a, b, c, d, e, 5); R0(e, a, b, c, d, 6); R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8); R0(b, c, d, e, a, 9); R0(a, b, c, d, e, 10); R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12); R0(c, d, e, a, b, 13); R0(b, c, d, e, a, 14); R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16); R1(d, e, a, b, c, 17); R1(c, d, e, a, b, 18); R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20); R2(e, a, b, c, d, 21); R2(d, e, a, b, c, 22); R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24); R2(a, b, c, d, e, 25); R2(e, a, b, c, d, 26); R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28); R2(b, c, d, e, a, 29); R2(a, b, c, d, e, 30); R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32); R2(c, d, e, a, b, 33); R2(b, c, d, e, a, 34); R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36); R2(d, e, a, b, c, 37); R2(c, d, e, a, b, 38); R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40); R3(e, a, b, c, d, 41); R3(d, e, a, b, c, 42); R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44); R3(a, b, c, d, e, 45); R3(e, a, b, c, d, 46); R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48); R3(b, c, d, e, a, 49); R3(a, b, c, d, e, 50); R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52); R3(c, d, e, a, b, 53); R3(b, c, d, e, a, 54); R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56); R3(d, e, a, b, c, 57); R3(c, d, e, a, b, 58); R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60); R4(e, a, b, c, d, 61); R4(d, e, a, b, c, 62); R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64); R4(a, b, c, d, e, 65); R4(e, a, b, c, d, 66); R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68); R4(b, c, d, e, a, 69); R4(a, b, c, d, e, 70); R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72); R4(c, d, e, a, b, 73); R4(b, c, d, e, a, 74); R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76); R4(d, e, a, b, c, 77); R4(c, d, e, a, b, 78); R4(b, c, d, e, a, 79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;

    memset(block, 0, 64);
}

/* SHA1Reset - Initialize new context */

void SHA1Reset(SHA1_CTX *context){
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Input(SHA1_CTX* context, BYTE *_data, DWORD len){
    DWORD i, j;
    BYTE *data = _data;

    j = (context->count[0] >> 3) & 63;
    if((context->count[0] += len << 3) < (len << 3))
        context->count[1]++;
    context->count[1] += (len >> 29);
    if((j + len) > 63){
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);
        for(; i + 63 < len; i += 64){
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);

}


/* Add padding and return the message digest. */

void SHA1Result(SHA1_CTX *context, BYTE *digest){
    DWORD i;
    BYTE finalcount[8];

    for(i = 0; i < 8; i++){
        finalcount[i] = (BYTE)
            ((context->count[(i >= 4 ? 0 : 1)] >>
            ((3 - (i & 3)) * 8)) & 255);  /* Endian independent */
    }
    SHA1Input(context, (BYTE *) "\200", 1);
    while((context->count[0] & 504) != 448){
        SHA1Input(context, (BYTE *) "\0", 1);
    }
    SHA1Input(context, finalcount, 8);  /* Should cause a SHA1Transform()
                                        */
    for(i = 0; i < 20; i++){
        digest[i] = (BYTE)
            ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) &
             255);
    }
    /* Wipe variables */
    i = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(finalcount, 0, 8);
}
/**************************************************************************
* NOTES:       Test Vectors (from FIPS PUB 180-1) to verify implementation
*              1- Input : "abc"
*              Output : A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
*              2- Input : "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
*              Output : 84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
*              2- Input : A million repetitions of 'a' - not applied (memory shortage)
*              Output : 34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*              More test vectors can be obtained from FIPS web site
***************************************************************************/
void SHA1_Perform(BYTE *indata, DWORD inlen, BYTE *outdata) //计算SHA-1的API   
{
    SHA1_CTX sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, indata, inlen);
    SHA1Result(&sha, outdata);
}
void SHA1_Perform2(BYTE *indata, DWORD inlen, BYTE *indata2, DWORD inlen2, BYTE *outdata){
    SHA1_CTX sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, indata, inlen);
    SHA1Input(&sha, indata2, inlen2);
    SHA1Result(&sha, outdata);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
* Number of 32-bit words comprising the Cipher Key. For this
* standard, Nk = 4, 6, or 8.
*/
int Nk;

/*
* Number of rounds, which is a function of  Nk  and  Nb (which is
* fixed). For this standard, Nr = 10, 12, or 14.
*/
int Nr;

/*
* S-box transformation table
*/
static const char *s_box[100] = {
    // 0     1     2     3     4     5     6     7     8     9
    "23", "34", "39", "20", "26", "92", "11", "54", "64", "52", //0
    "13", "63", "17", "60", "94", "32", "33", "89", "93", "00", //1
    "68", "37", "98", "87", "81", "79", "31", "99", "14", "90", //2
    "83", "69", "49", "96", "56", "21", "58", "71", "01", "95", //3
    "85", "38", "70", "74", "61", "12", "55", "30", "07", "19", //4
    "91", "45", "27", "50", "06", "02", "24", "75", "62", "66", //5
    "08", "51", "41", "35", "86", "88", "15", "80", "40", "18", //6
    "77", "53", "46", "22", "47", "05", "67", "76", "59", "48", //7
    "09", "78", "36", "43", "57", "03", "04", "42", "97", "29", //8
    "73", "44", "84", "72", "82", "25", "65", "10", "16", "28" };//9

                                                                 /*
                                                                 * Inverse S-box transformation table
                                                                 */
static const char *inv_s_box[100] = {
    // 0     1     2     3     4     5     6     7     8     9
    "19", "38", "55", "85", "86", "75", "54", "48", "60", "80", //0
    "97", "06", "45", "10", "28", "66", "98", "12", "69", "49", //1
    "03", "35", "73", "00", "56", "95", "04", "52", "99", "89", //2
    "47", "26", "15", "16", "01", "63", "82", "21", "41", "02", //3
    "68", "62", "87", "83", "91", "51", "72", "74", "79", "32", //4
    "53", "61", "09", "71", "07", "46", "34", "84", "36", "78", //5
    "13", "44", "58", "11", "08", "96", "59", "76", "20", "31", //6
    "42", "37", "93", "90", "43", "57", "77", "70", "81", "25", //7
    "67", "24", "94", "30", "92", "40", "64", "23", "65", "17", //8
    "29", "50", "05", "18", "14", "39", "33", "88", "22", "27" };//9



void add_round_key(char *state, char *w, uint8_t r){

    uint8_t c;

    for(c = 0; c < 4; c++){
        state[4 * 0 + c] = (state[4 * 0 + c] - '0' + w[4 * 4 * r + 4 * c + 0] - '0') % 10 + '0';
        state[4 * 1 + c] = (state[4 * 1 + c] - '0' + w[4 * 4 * r + 4 * c + 1] - '0') % 10 + '0';
        state[4 * 2 + c] = (state[4 * 2 + c] - '0' + w[4 * 4 * r + 4 * c + 2] - '0') % 10 + '0';
        state[4 * 3 + c] = (state[4 * 3 + c] - '0' + w[4 * 4 * r + 4 * c + 3] - '0') % 10 + '0';

    }
}

void inv_add_round_key(char *state, char *w, uint8_t r){
    uint8_t c, i;

    for(c = 0; c < 4; c++){
        for(i = 0; i < 4; i++){
            if(state[4 * i + c] >= w[4 * 4 * r + 4 * c + i]){
                state[4 * i + c] = state[4 * i + c] - w[4 * 4 * r + 4 * c + i] + '0';
            }
            else{
                state[4 * i + c] = state[4 * i + c] - w[4 * 4 * r + 4 * c + i] + 10 + '0';
            }
        }
    }

}

/*
* S盒替换
*/
void sub_bytes(char *state){
    for(uint8_t i = 0; i < 8; i++){
        uint8_t row = state[i * 2] - '0';
        uint8_t col = state[i * 2 + 1] - '0';
        const char *box = s_box[row * 10 + col];
        state[i * 2] = box[0];
        state[i * 2 + 1] = box[1];
    }
}

void inv_sub_bytes(char *state){
    for(uint8_t i = 0; i < 8; i++){
        uint8_t row = state[i * 2] - '0';
        uint8_t col = state[i * 2 + 1] - '0';
        const char *box = inv_s_box[row * 10 + col];
        state[i * 2] = box[0];
        state[i * 2 + 1] = box[1];
    }
}

void shift_rows(char *state){

    uint8_t i, k, s, tmp;

    for(i = 1; i < 4; i++){
        // shift(1,4)=1; shift(2,4)=2; shift(3,4)=3
        // shift(r, 4) = r;
        s = 0;
        while(s < i){
            tmp = state[4 * i + 0];

            for(k = 1; k < 4; k++){
                state[4 * i + k - 1] = state[4 * i + k];
            }

            state[4 * i + 4 - 1] = tmp;
            s++;
        }
    }
}

void inv_shift_rows(char *state){

    uint8_t i, k, s, tmp;

    for(i = 1; i < 4; i++){
        s = 0;
        while(s < i){
            tmp = state[4 * i + 4 - 1];

            for(k = 4 - 1; k > 0; k--){
                state[4 * i + k] = state[4 * i + k - 1];
            }

            state[4 * i + 0] = tmp;
            s++;
        }
    }
}

void coef_mult(uint8_t *a, uint8_t *b, uint8_t *res){
    res[0] = (a[0] * b[0] + a[3] * b[1] + a[2] * b[2] + a[1] * b[3]) % 10;
    res[1] = (a[1] * b[0] + a[0] * b[1] + a[3] * b[2] + a[2] * b[3]) % 10;
    res[2] = (a[2] * b[0] + a[1] * b[1] + a[0] * b[2] + a[3] * b[3]) % 10;
    res[3] = (a[3] * b[0] + a[2] * b[1] + a[1] * b[2] + a[0] * b[3]) % 10;
}

void mix_columns(char *state){

    uint8_t a[] = { 5, 1, 2, 3 };
    uint8_t i, j, col[4], res[4];

    for(j = 0; j < 4; j++){
        for(i = 0; i < 4; i++){
            col[i] = state[4 * i + j] - '0';
        }

        coef_mult(a, col, res);

        for(i = 0; i < 4; i++){
            state[4 * i + j] = res[i] + '0';
        }
    }
}

void inv_mix_columns(char *state){

    uint8_t a[] = { 5, 3, 4, 9 }; // a(x) = {0e} + {09}x + {0d}x2 + {0b}x3
    uint8_t i, j, col[4], res[4];

    for(j = 0; j < 4; j++){
        for(i = 0; i < 4; i++){
            col[i] = state[4 * i + j] - '0';
        }

        coef_mult(a, col, res);

        for(i = 0; i < 4; i++){
            state[4 * i + j] = res[i] + '0';
        }
    }
}

uint8_t R[] = { 0, 0, 0, 0 };

uint8_t *Rcon(uint8_t i){
    R[0] = i + 1;
    return R;
}

void coef_add(uint8_t *a, uint8_t *b, uint8_t *d){
    d[0] = (a[0] + b[0]) % 10;
    d[1] = (a[1] + b[1]) % 10;
    d[2] = (a[2] + b[2]) % 10;
    d[3] = (a[3] + b[3]) % 10;
}

void sub_word(uint8_t *w){

    uint8_t i;

    for(i = 0; i < 2; i++){
        uint8_t row = w[i * 2];
        uint8_t col = w[i * 2 + 1];
        const char *box = s_box[row * 10 + col];
        w[i * 2] = box[0] - '0';
        w[i * 2 + 1] = box[1] - '0';
    }
}

void rot_word(uint8_t *w){

    uint8_t tmp;
    uint8_t i;

    tmp = w[0];

    for(i = 0; i < 3; i++){
        w[i] = w[i + 1];
    }

    w[3] = tmp;
}

void key_expansion(const char *key, char *w){
    uint8_t tmp[4];
    uint8_t i;
    uint8_t len = 4 * (Nr + 1);

    for(i = 0; i < Nk; i++){
        w[4 * i + 0] = key[4 * i + 0] - '0';
        w[4 * i + 1] = key[4 * i + 1] - '0';
        w[4 * i + 2] = key[4 * i + 2] - '0';
        w[4 * i + 3] = key[4 * i + 3] - '0';
    }

    for(i = 4; i < len; i++){
        tmp[0] = w[4 * (i - 1)];
        tmp[1] = w[4 * (i - 1) + 1];
        tmp[2] = w[4 * (i - 1) + 2];
        tmp[3] = w[4 * (i - 1) + 3];

        if(i % 4 == 0){
            rot_word(tmp);
            coef_add(tmp, Rcon(i / 4), tmp);
            sub_word(tmp);
        }

        //W[i] = W[i-Nk] + tmp mod 10;
        w[4 * i + 0] = (w[4 * (i - 4) + 0] + tmp[0]) % 10;
        w[4 * i + 1] = (w[4 * (i - 4) + 1] + tmp[1]) % 10;
        w[4 * i + 2] = (w[4 * (i - 4) + 2] + tmp[2]) % 10;
        w[4 * i + 3] = (w[4 * (i - 4) + 3] + tmp[3]) % 10;
    }

    for(i = 0; i < len * 4; i++){
        w[i] = w[i] + '0';
    }
}

void encrypt(const char *in, const char *key, char *out){
    char state[4 * 4];
    uint8_t r, i, j;

    char *w;

    switch(strlen(key)){
    case 16:
        Nk = 4;
        Nr = 10;
        break;
    case 24:
        Nk = 6;
        Nr = 12;
        break;
    case 32:
        Nk = 8;
        Nr = 14;
        break;
    default:
        return;
    }
    w = (char *)malloc(4 * (Nr + 1) * 4 + 1);
    w[4 * (Nr + 1) * 4 + 1] = '\0';
    key_expansion(key, w);

    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            state[4 * i + j] = in[4 * i + j];
        }
    }

    add_round_key(state, w, 0);
    for(r = 1; r < Nr; r++){
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, w, r);
    }
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, w, Nr);

    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            out[4 * i + j] = state[4 * i + j];
        }
    }
    out[16] = '\0';
}

void decrypt(const char *in, const char *key, char *out){
    char state[4 * 4];
    uint8_t r, i, j;

    char *w;

    switch(strlen(key)){
    case 16:
        Nk = 4;
        Nr = 10;
        break;
    case 24:
        Nk = 6;
        Nr = 12;
        break;
    case 32:
        Nk = 8;
        Nr = 14;
        break;
    default:
        return;
    }
    w = (char *)malloc(4 * (Nr + 1) * 4 + 1);
    w[4 * (Nr + 1) * 4 + 1] = '\0';
    key_expansion(key, w);

    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            state[4 * i + j] = in[4 * i + j];
        }
    }

    inv_add_round_key(state, w, Nr);

    for(r = Nr - 1; r >= 1; r--){
        inv_shift_rows(state);
        inv_sub_bytes(state);
        inv_add_round_key(state, w, r);
        inv_mix_columns(state);
    }

    inv_shift_rows(state);
    inv_sub_bytes(state);
    inv_add_round_key(state, w, 0);

    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            out[4 * i + j] = state[4 * i + j];
        }
    }
    out[16] = '\0';
}

bool Basic_AES10_encrypt(const char* pIn, char* pOut, long lDatalen, const char* pKey){
    if(!(pOut && pIn && pKey && lDatalen % 16 == 0)){
        return false;
    }
    for(int i = 0; i < lDatalen; i += 16){
        encrypt(pIn + i, pKey, pOut + i);
    }
    return true;
}

//TODO 发布时去掉解析的方法，提高安全性
bool Basic_AES10_decrypt(const char* pIn, char* pOut, long lDatalen, const char* pKey){
    if(!(pOut && pIn && pKey && lDatalen % 16 == 0)){
        return false;
    }
    for(int i = 0; i < lDatalen; i += 16){
        decrypt(pIn + i, pKey, pOut + i);
    }
    return true;
}


__NS_BASIC_END


