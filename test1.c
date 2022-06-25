#include <stdio.h>
#include <string.h>

typedef struct {
   int face;
   double u, v;
} FUV;
int main()
{
    
    static FUV t[2688][5376];
    FILE *fp = fopen( "filename.bin", "wb" );
    for (int i = 0; i < 2688;++i){
        for (int j = 0; j < 5376;++j){
            t[i][j].face = 3;
            t[i][j].u = 4.0;
            t[i][j].v = 5.0;
        }
    }
    fwrite(t, sizeof(FUV), 2688*5376, fp);
    fclose(fp);
    FILE *fpread = fopen( "filename.bin", "rb" );
    static FUV a[2688][5376];
    fread(a, sizeof(FUV), 2688*5376, fpread);
    // printf("%d\n", a[2][2].face);
    // printf("%lf\n", a[2][1].u);
    // printf("%lf\n", a[0][0].v);

    return 0;
}
// int main()
// {
//     bool t[2688][5376];
//     return 0;
// }