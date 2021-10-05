/*
 * このＣコードは次のURLに掲載されているソースコードを取り込み、
 * コメントと main() を削除したものである。
 * https://blog.goo.ne.jp/masaki_goo_2006/e/754f937f81f8b3ac389b9265c52fd98d
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CASE        break;case
#define DEFAULT     break;default

static long funcAddSub();

#define BLKTOP      '('
#define BLKEND      ')'
#define SPC         ' '
#define TAB         '\t'

enum eERRCODE {
    ERRCODE_SUCCESS,        // 正常
    ERRCODE_FACTOR,         // 未サポートの演算子です。
    MAX_ERRCODE,
};

static const char *g_parse;
static       long  g_error;

static long
funcValue()
{
    return strtol( g_parse, (char**)&g_parse, 10 );
}

static long
funcFactor()
{
    long ans;
    
    while ( (*g_parse == SPC) || (*g_parse == TAB) ){
        g_parse++;
    }
    if ( *g_parse == BLKTOP ){
        g_parse++;
        ans = funcAddSub();
        g_parse++;
        return ans;
    }
    if ( isdigit(*g_parse) || (*g_parse == '+') || (*g_parse == '-') ){
        return funcValue();
    }
    g_error = ERRCODE_FACTOR;
    return 0;
}

static long
funcMulDiv()
{
    long ans = funcFactor();
    
    for ( ; ; ){
        switch ( *g_parse ){
            CASE '*':       g_parse++; ans *= funcFactor();
            CASE '/':       g_parse++; ans /= funcFactor();
            CASE '%':       g_parse++; ans %= funcFactor();
            CASE SPC:       g_parse++;
            CASE TAB:       g_parse++;
            DEFAULT:        return ans;
        }
    }
}

static long
funcAddSub()
{
    long ans = funcMulDiv();
    
    for ( ; ; ){
        switch ( *g_parse ){
            CASE '+':       g_parse++; ans += funcMulDiv();
            CASE '-':       g_parse++; ans -= funcMulDiv();
            CASE SPC:       g_parse++;
            CASE TAB:       g_parse++;
            DEFAULT:        return ans;
        }
    }
}

long
calcExpress(const char *string)
{
    g_parse = string;
    g_error = ERRCODE_SUCCESS;
    return funcAddSub();
}

long
calcErrCode()
{
    return g_error;
}

#if 0
int main( void )
{
    char    buff[ 256 ];
    char*   find;
    
    while ( fgets(buff,sizeof(buff),stdin) != NULL ){
        if ( (find = strchr(buff,'\n')) != NULL ){
            *find = '\0';
        }
        if ( buff[0] != '\0' ){
            printf( "%s = %ld\n", buff, calcExpress(buff) );
            
            switch ( calcErrCode() ){
                CASE ERRCODE_FACTOR:    printf( "ERRCODE_FACTOR\n\n" );
                DEFAULT:                printf( "\n" );
            }
        }
    }
    return 0;
}
#endif
