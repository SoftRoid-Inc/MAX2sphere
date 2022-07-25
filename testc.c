#include <dirent.h>
#include <omp.h>
#include <unistd.h>

#include "MAX2sphere.h"
#define PATHNAME_SIZE 512

/*
        Convert a pair of frames from the GoPro Max camera to an equirectangular
        Sept 08: First version based upon cube2sphere
        Sept 10: Added output file mask
*/
// 高速化について
// https://www.notion.so/softroid/MAX2sphere-b899aae68c11426682e8dc494751ecaf
PARAMS params;

// These are known frame templates
// The appropaite one to use will be auto detected, error is none match
#define NTEMPLATE 2
// #define TABLEPATH "./precalc.bin"

//対応可能な360画像のサイズは以下の2種類
// 4096×1344か2272×736以外の場合は、以下のtemplateに追記が必要
FRAMESPECS template[NTEMPLATE] = {{4096, 1344, 1376, 1344, 32, 5376},
                                  {2272, 736, 768, 736, 16, 2944}};
int whichtemplate = -1;  // Which frame template do we thnk we have

int main(int argc, char **argv) {
    // テーブルの読み込み
    Init();
    static FUV arr_read[2688][5376]
                       [4];  // TODO: 要素数を変数で指定できるように変更
    int unreadable = 0;  // 360画像のサイズが4096×1344でない場合に1となる変数
    // if (whichtemplate != 0) {
    //     fprintf(stderr, "Size doesn't match with Pre-calculation table\n");
    //     unreadable = 1;
    // }
    
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            params.outwidth = atoi(argv[i + 1]);
            params.outwidth = 4 * (params.outwidth / 4);  // Make factor of 4
            params.outheight = params.outwidth / 2;       // Will be even
        } else if (strcmp(argv[i], "-a") == 0) {
            params.antialias = MAX(1, atoi(argv[i + 1]));
            params.antialias2 = params.antialias * params.antialias;
        } else if (strcmp(argv[i], "-o") == 0) {
            strcpy(params.outfilename, argv[i + 1]);
        } else if (strcmp(argv[i], "-d") == 0) {
            params.debug = TRUE;
        } else if (strcmp(argv[i], "-r") == 0) {
            strcpy(params.readprecalctable, argv[i + 1]);
        } else if (strcmp(argv[i], "-m") == 0) {
            params.writeprecalctable = TRUE;
        }
    }
    FILE *fpread;
    if (strlen(params.readprecalctable) > 0) {
        if ((fpread = fopen(params.readprecalctable, "rb")) == NULL) {
            fprintf(stderr,
                    "CheckPrecalculationFile() - Failed to open binfile "
                    "\"%s\"\n",  // binファイルが読み込めない時に出力
                    params.readprecalctable);
            return (-1);
        } else if (fread(arr_read, sizeof(FUV), 2688 * 5376 * 4, fpread) <
                   2688 * 5376 * 4)  // 360画像のサイズに応じて変更が必要
        {
            fprintf(
                stderr,
                "binfile is broken() - mismatch size \n");  // binファイルを読み込んだが、メモリなどの原因で読み込んだ配列が小さい時に出力
            return (-1);
        }
        fclose(fpread);
    }

    DIR *dir;
    struct dirent *dp;
    char dirpath1[] = "../../8/130/2022/04/27/02:34:31.000000/prepro/images/track0";
    char dirpath2[] = "../../8/130/2022/04/27/02:34:31.000000/prepro/images/track5";
    const char outdir[] = "../../8/130/2022/04/27/02:34:31.000000/prepro/images/er";
    dir = opendir(dirpath1);
    if (dir == NULL) {
        return 1;
    }

    // dp = readdir(dir);
    char pathname[PATHNAME_SIZE];
    memset(pathname, '\0', PATHNAME_SIZE);
    if(getcwd(pathname, PATHNAME_SIZE)==NULL){
        fprintf(stderr, "fail to getcwd");
    };
    // fprintf(stdout, "現在のファイルパス:%s\n", pathname);
    dp = readdir(dir);
    char pathlist[5000][256];
    int cnt = 0;
    for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
        char fname[256];
        strcpy(fname, dp->d_name);
        if(strlen(fname)<3){
            continue;
        }
        strcpy(pathlist[cnt], fname);
        cnt += 1;
    }
    // for (int i = 0; i < 50; ++i) fprintf(stdout, "%s\n", pathlist[i]);
    int total = 0;
    #pragma omp parallel for
    for (int cnti = 0; cnti < cnt;cnti++) {
        char fname[256];
        strcpy(fname, pathlist[cnti]);
        char fname1[512],fname2[512],fname3[512];
        sprintf(fname1, "%s/%s", dirpath1, fname);
        sprintf(fname2, "%s/%s", dirpath2, fname);
        sprintf(fname3, "%s/%s", outdir, fname);

        // fprintf(stdout, "%s", str3);
        int i, j, index, face, aj, ai;
        double x, y, longitude, latitude;
        UV uv;
        COLOUR16 csum, czero = {0, 0, 0};
        BITMAP4 c;
        BITMAP4 *frame1 = NULL, *frame2 = NULL, *spherical = NULL;
        // Malloc images
        if ((whichtemplate = CheckFrames(fname1,
        fname2,&params.framewidth,&params.frameheight)) < 0)
            exit(-1);
        frame1 = Create_Bitmap(params.framewidth,
        params.frameheight); frame2 =
        Create_Bitmap(params.framewidth, params.frameheight); if
        (params.outwidth < 0) {
            params.outwidth = template[whichtemplate].equiwidth;
            params.outheight = params.outwidth / 2;
        }
        spherical = Create_Bitmap(params.outwidth, params.outheight);
        // if (frame1 == NULL || frame2 == NULL || spherical == NULL) {
        //     fprintf(stderr, "%s() - Failed to malloc memory for the images\n",argv[0]);
        //     exit(-1);
        // }
        if (!ReadFrame(frame1, fname1,
        params.framewidth,params.frameheight))
            exit(-1);
        if (!ReadFrame(frame2, fname2,
        params.framewidth,params.frameheight))
            exit(-1);
        // fprintf(stdout, "%s\n", outdir);

        //Form the spherical map
        for (j = 0; j < params.outheight; j++) {
            // if (params.debug && j % (params.outheight / 32) == 0)
            //     fprintf(stderr, "%s() - Scan line %d\n", argv[0],j); 
            for (i = 0; i < params.outwidth; i++) {
                csum = czero;  // Supersampling antialising sum

                // Antialiasing loops
                // printf("%d", params.antialias);
                for (ai = 0; ai < params.antialias; ai++) {
                    x = (i + ai / (double)params.antialias) /
                        (double)params.outwidth;  // 0 ... 1
                    for (aj = 0; aj < params.antialias; aj++) {
                        y = (j + aj / (double)params.antialias) /
                                (double)params.outheight -0.5;  // -0.5 ... 0.5
                        int antinum = ai * params.antialias + aj;
                        if (strlen(params.readprecalctable) == 0 ||
                            unreadable == 1) {
                            longitude = x * TWOPI - M_PI;  // -pi ...pi 
                            latitude = y * M_PI;           //-pi/2 ... pi/2 
                            if ((face =FindFaceUV(longitude, latitude, &uv)) <0) continue;
                            // テーブルを作成するためのコード
                            if (params.writeprecalctable) {
                                arr_read[j][i][antinum].u = uv.u;
                                arr_read[j][i][antinum].v = uv.v;
                                arr_read[j][i][antinum].face = face;
                            }

                        } else {
                            face = arr_read[j][i][antinum].face;
                            uv.u = (double)arr_read[j][i][antinum].u;
                            uv.v = (double)arr_read[j][i][antinum].v;
                        }
                        c = GetColour(face, uv, frame1, frame2);
                        csum.r += c.r;
                        csum.g += c.g;
                        csum.b += c.b;
                    }
                }
                // Finally update the spherical image
                index = j * params.outwidth + i;
                spherical[index].r = csum.r / params.antialias2;
                spherical[index].g = csum.g / params.antialias2;
                spherical[index].b = csum.b / params.antialias2;
            }
        }
        // if (params.debug)
        //     fprintf(stderr, "%s() - Writing equirectangular\n",argv[0]);
        WriteSpherical(fname3, spherical, params.outwidth, params.outheight);
        // fprintf(stderr,"%d\n",cnti);
#pragma omp atomic
        total+=1;
        fprintf(stderr, "\r[%d / 5]", total);     
        free(frame1);
        free(frame2);
        free(spherical);   
    }
    closedir(dir);
    exit(0);
}

/*
        Check the frames
        - do they exist
        - are they jpeg
        - are they the same size
        - determine which frame template we are using
*/
int CheckFrames(char *fname1, char *fname2, int *width, int *height) {
    int i, n = -1;
    int w1, h1, w2, h2, depth;
    FILE *fptr;

    // if (!IsJPEG(fname1) || !IsJPEG(fname2)) {
    //    fprintf(stderr,"CheckFrames() - frame name does not look like a jpeg
    //    file\n"); return(-1);
    // }

    // Frame 1
    if ((fptr = fopen(fname1, "rb")) == NULL) {
        fprintf(stderr, "CheckFrames() - Failed to open first frame \"%s\"\n",
                fname1);
        return (-1);
    }
    JPEG_Info(fptr, &w1, &h1, &depth);
    fclose(fptr);

    // Frame 2
    if ((fptr = fopen(fname2, "rb")) == NULL) {
        fprintf(stderr, "CheckFrames() - Failed to open second frame \"%s\"\n",
                fname2);
        return (-1);
    }
    JPEG_Info(fptr, &w2, &h2, &depth);
    fclose(fptr);

    // Are they the same size
    if (w1 != w2 || h1 != h2) {
        fprintf(
            stderr,
            "CheckFrames() - Frame sizes don't match, %d != %d or %d != %d\n",
            w1, h1, w2, h2);
        return (-1);
    }

    // Is it a known template?
    for (i = 0; i < NTEMPLATE; i++) {
        if (w1 == template[i].width && h1 == template[i].height) {
            n = i;
            break;
        }
    }
    if (n < 0) {
        fprintf(stderr, "CheckFrames() - No recognised frame template\n");
        return (-1);
    }

    *width = w1;
    *height = h1;

    return (n);
}

/*
   Write spherical image
        The file name is either params.outfilename or based upon the basename
   provided
*/
int WriteSpherical(char *basename, BITMAP4 *img, int w, int h) {
    int i;
    FILE *fptr;
    char fname[256];

    // Create the output file name
    if (strlen(params.outfilename) < 1) {
        strcpy(fname, basename);
        for (i = strlen(fname) - 1; i > 0; i--) {
            if (fname[i] == '.') {
                fname[i] = '\0';
                break;
            }
        }
        strcat(fname, "_sphere.jpg");
    } else {
        strcpy(fname, params.outfilename);
    }

    // Save
    if ((fptr = fopen(fname, "wb")) == NULL) {
        fprintf(stderr,
                "WriteSpherical() - Failed to open output file \"%s\"\n",
                fname);
        return (FALSE);
    }
    JPEG_Write(fptr, img, w, h, 100);
    fclose(fptr);

    return (TRUE);
}

/*
   Read a frame
*/
int ReadFrame(BITMAP4 *img, char *fname, int w, int h) {
    FILE *fptr;

    if (params.debug)
        fprintf(stderr, "ReadFrame() - Reading image \"%s\"\n", fname);

    // Attempt to open file
    if ((fptr = fopen(fname, "rb")) == NULL) {
        fprintf(stderr, "ReadFrame() - Failed to open \"%s\"\n", fname);
        return (FALSE);
    }

    // Read image data
    if (JPEG_Read(fptr, img, &w, &h) != 0) {
        fprintf(stderr,
                "ReadFrame() - Failed to correctly read JPG file \"%s\"\n",
                fname);
        return (FALSE);
    }
    fclose(fptr);

    return (TRUE);
}

/*
   Given longitude and latitude find corresponding face id and (u,v) coordinate
   on the face Return -1 if something went wrong, shouldn't
*/
int FindFaceUV(double longitude, double latitude, UV *uv) {
    int k, found = -1;
    double mu, denom, coslatitude, fourdivpi;
    UV fuv;
    XYZ p, q;

    fourdivpi = 4.0 / M_PI;

    // p is the ray from the camera position into the scene
    coslatitude = cos(latitude);
    p.x = coslatitude * sin(longitude);
    p.y = coslatitude * cos(longitude);
    p.z = sin(latitude);

    // Find which face the vector intersects
    for (k = 0; k < 6; k++) {
        denom = -(params.faces[k].a * p.x + params.faces[k].b * p.y +
                  params.faces[k].c * p.z);

        // Is p parallel to face? Shouldn't ever happen.
        // if (ABS(denom) < 0.000001)
        //   continue;

        // Find position q along ray and ignore intersections on the back
        // pointing ray?
        if ((mu = params.faces[k].d / denom) < 0) continue;
        q.x = mu * p.x;
        q.y = mu * p.y;
        q.z = mu * p.z;

        // Find out which face it is on
        switch (k) {
            case LEFT:
            case RIGHT:
                if (q.y <= 1 && q.y >= -1 && q.z <= 1 && q.z >= -1) found = k;
                q.y = atan(q.y) * fourdivpi;
                q.z = atan(q.z) * fourdivpi;
                break;
            case FRONT:
            case BACK:
                if (q.x <= 1 && q.x >= -1 && q.z <= 1 && q.z >= -1) found = k;
                q.x = atan(q.x) * fourdivpi;
                q.z = atan(q.z) * fourdivpi;
                break;
            case TOP:
            case DOWN:
                if (q.x <= 1 && q.x >= -1 && q.y <= 1 && q.y >= -1) found = k;
                q.x = atan(q.x) * fourdivpi;
                q.y = atan(q.y) * fourdivpi;
                break;
        }
        if (found >= 0) break;
    }
    if (found < 0 || found > 5) {
        fprintf(stderr,
                "FindFaceUV() - Didn't find an intersecting face, shouldn't "
                "happen!\n");
        return (-1);
    }

    // Determine the u,v coordinate
    switch (found) {
        case LEFT:
            fuv.u = q.y + 1;
            fuv.v = q.z + 1;
            break;
        case RIGHT:
            fuv.u = 1 - q.y;
            fuv.v = q.z + 1;
            break;
        case FRONT:
            fuv.u = q.x + 1;
            fuv.v = q.z + 1;
            break;
        case BACK:
            fuv.u = 1 - q.x;
            fuv.v = q.z + 1;
            break;
        case DOWN:
            fuv.u = 1 - q.x;
            fuv.v = 1 - q.y;
            break;
        case TOP:
            fuv.u = 1 - q.x;
            fuv.v = q.y + 1;
            break;
    }
    fuv.u *= 0.5;
    fuv.v *= 0.5;

    // Need to understand this at some stage
    if (fuv.u >= 1) fuv.u = NEARLYONE;
    if (fuv.v >= 1) fuv.v = NEARLYONE;

    if (fuv.u < 0 || fuv.v < 0 || fuv.u >= 1 || fuv.v >= 1) {
        fprintf(stderr,
                "FindFaceUV() - Illegal (u,v) coordinate (%g,%g) on face %d\n",
                fuv.u, fuv.v, found);
        return (-1);
    }

    *uv = fuv;

    return (found);
}

/*
        Given a face and a (u,v) in that face, determine colour from the two
   frames This is largely a mapping excercise from (u,v) of each face to the two
   frames For faces left, right, down and top a blend is required between the
   two halves Relies on the values from the frame template
*/
BITMAP4 GetColour(int face, UV uv, BITMAP4 *frame1, BITMAP4 *frame2) {
    int ix, iy, index;
    int x0, w;
    double alpha, duv;
    UV uvleft, uvright;
    BITMAP4 c = {0, 0, 0}, c1, c2;

    // Rotate u,v counterclockwise by 90 degrees for lower frame
    if (face == DOWN || face == BACK || face == TOP) RotateUV90(&uv);

    // v doesn't change
    uvleft.v = uv.v;
    uvright.v = uv.v;

    switch (face) {
        // Frame 1
        case FRONT:
        case BACK:
            x0 = template[whichtemplate].sidewidth;
            w = template[whichtemplate].centerwidth;
            ix = x0 + uv.u * w;
            iy = uv.v * template[whichtemplate].height;
            index = iy * template[whichtemplate].width + ix;
            c = (face == FRONT) ? frame1[index] : frame2[index];
            break;
        case LEFT:
        case DOWN:
            w = template[whichtemplate].sidewidth;
            duv = template[whichtemplate].blendwidth / (double)w;
            uvleft.u = 2 * (0.5 - duv) * uv.u;
            uvright.u = 2 * (0.5 - duv) * (uv.u - 0.5) + 0.5 + duv;
            if (uvleft.u <= 0.5 - 2 * duv) {
                ix = uvleft.u * w;
                iy = uvleft.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c = (face == LEFT) ? frame1[index] : frame2[index];
            } else if (uvright.u >= 0.5 + 2 * duv) {
                ix = uvright.u * w;
                iy = uvright.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c = (face == LEFT) ? frame1[index] : frame2[index];
            } else {
                ix = uvleft.u * w;
                iy = uvleft.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c1 = (face == LEFT) ? frame1[index] : frame2[index];
                ix = uvright.u * w;
                iy = uvright.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c2 = (face == LEFT) ? frame1[index] : frame2[index];
                alpha = (uvleft.u - 0.5 + 2 * duv) / (2 * duv);
                c = ColourBlend(c1, c2, alpha);
            }
            break;
        case RIGHT:
        case TOP:
            x0 = template[whichtemplate].sidewidth +
                 template[whichtemplate].centerwidth;
            w = template[whichtemplate].sidewidth;
            duv = template[whichtemplate].blendwidth / (double)w;
            uvleft.u = 2 * (0.5 - duv) * uv.u;
            uvright.u = 2 * (0.5 - duv) * (uv.u - 0.5) + 0.5 + duv;
            if (uvleft.u <= 0.5 - 2 * duv) {
                ix = x0 + uvleft.u * w;
                iy = uv.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c = (face == RIGHT) ? frame1[index] : frame2[index];
            } else if (uvright.u >= 0.5 + 2 * duv) {
                ix = x0 + uvright.u * w;
                iy = uvright.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c = (face == RIGHT) ? frame1[index] : frame2[index];
            } else {
                ix = x0 + uvleft.u * w;
                iy = uvleft.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c1 = (face == RIGHT) ? frame1[index] : frame2[index];
                ix = x0 + uvright.u * w;
                iy = uvright.v * template[whichtemplate].height;
                index = iy * template[whichtemplate].width + ix;
                c2 = (face == RIGHT) ? frame1[index] : frame2[index];
                alpha = (uvleft.u - 0.5 + 2 * duv) / (2 * duv);
                c = ColourBlend(c1, c2, alpha);
            }
            break;
    }

    return (c);
}

/*
        Blend two colours
*/
BITMAP4 ColourBlend(BITMAP4 c1, BITMAP4 c2, double alpha) {
    BITMAP4 c;

    c.r = (1 - alpha) * c1.r + alpha * c2.r;
    c.g = (1 - alpha) * c1.g + alpha * c2.g;
    c.b = (1 - alpha) * c1.b + alpha * c2.b;

    return (c);
}

/*
        Rotate a uv by 90 degrees counterclockwise
*/
void RotateUV90(UV *uv) {
    UV tmp;

    tmp = *uv;
    uv->u = tmp.v;
    uv->v = NEARLYONE - tmp.u;
}

/*
        Initialise parameters structure
*/
void Init(void) {
    params.outwidth = -1;
    params.outheight = -1;
    params.framewidth = -1;
    params.frameheight = -1;
    params.antialias = 2;
    params.antialias2 = 4;  // antialias squared
    params.outfilename[0] = '\0';
    params.debug = FALSE;

    // Parameters for the 6 cube planes, ax + by + cz + d = 0
    params.faces[LEFT].a = -1;
    params.faces[LEFT].b = 0;
    params.faces[LEFT].c = 0;
    params.faces[LEFT].d = -1;

    params.faces[RIGHT].a = 1;
    params.faces[RIGHT].b = 0;
    params.faces[RIGHT].c = 0;
    params.faces[RIGHT].d = -1;

    params.faces[TOP].a = 0;
    params.faces[TOP].b = 0;
    params.faces[TOP].c = 1;
    params.faces[TOP].d = -1;

    params.faces[DOWN].a = 0;
    params.faces[DOWN].b = 0;
    params.faces[DOWN].c = -1;
    params.faces[DOWN].d = -1;

    params.faces[FRONT].a = 0;
    params.faces[FRONT].b = 1;
    params.faces[FRONT].c = 0;
    params.faces[FRONT].d = -1;

    params.faces[BACK].a = 0;
    params.faces[BACK].b = -1;
    params.faces[BACK].c = 0;
    params.faces[BACK].d = -1;
}

/*
   Time scale at microsecond resolution but returned as seconds
        OS dependent, an alternative will need to be found for non UNIX systems
*/
double GetRunTime(void) {
    double sec = 0;
    struct timeval tp;

    gettimeofday(&tp, NULL);
    sec = tp.tv_sec + tp.tv_usec / 1000000.0;

    return (sec);
}

/*
   Standard usage string
*/
void GiveUsage(char *s) {
    fprintf(stderr, "Usage: %s [options] track0filename track5filename\n", s);
    fprintf(stderr, "Options\n");
    fprintf(stderr, "   -w n      sets the output image width, default: %d\n",
            params.outwidth);
    fprintf(stderr, "   -a n      sets antialiasing level, default = %d\n",
            params.antialias);
    fprintf(stderr,
            "   -o s      specify the output filename, default is based on "
            "track0 name\n");
    fprintf(stderr, "   -d        enable debug mode, default: off\n");
    fprintf(stderr,
            "   -r        enable read pre-calculation mode and specify table "
            "path\n");
    fprintf(stderr, "   -m        memorize calculation mode, default: off\n");
    exit(-1);
}
