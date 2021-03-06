# MAX2Sphere

Takes raw GoPro .360 frames (two strips of cube map projections encoded as EAC) and converts them to a more widely recognised equirectangular projection.
## 高速化の変更点
・MAX2sphere_precalc.cの追加
・MAX2sphere.hの変更
・Makefileの変更
## 仕様変更
ER画像と.360画像の画素位置の対応関係を前計算する方式に変更。
### 前計算テーブルについて
関数FindFaceUVを用いて、ER画像の各画素(i,j)に対して.360ファイルの面と位置(f,u,v)が計算される。
このときアンチエイリアス処理が行われるため、実際には各(i,j)に対して(f,u,v)が4セット求められる。
したがって、H×WサイズのER画像においてはH×W×4×3(f,u,v)の値を前計算テーブルに保存している。
尚、3次元配列H×W×4の各要素に、MAX2sphrere.hで定義したデータ構造FUVが保存されている。

### MAX2sphere_precalc内の主な変更点
95行目 ~ 121行目：前計算テーブルの読み込み(-rオプション)\
145行目 ~ 149行目：前計算テーブルの作成(-mオプション)\
151行目 ~ 155行目：前計算テーブルの値を利用(-rオプション)\
170行目 ~ 174行目：前計算テーブルの出力(-mオプション) 
## 使い方
1. ビルド\
下記コマンドでビルドされるのは**MAX2sphere_precalc**に変更されているので注意
```
make -f Makefile
```
2. 実行
```
./MAX2sphere_precalc -r テーブルファイルのパス -o 出力ファイルのパス track0ファイルのパス track5ファイルのパス
```
3. (補足) pythonでコマンドを叩く場合の例
```
command = f"./MAX2sphere_precalc -r './precalc.bin' -o {output_filepath} {track0_image_path} {track5_image_path}"
```
## オプション
追加したオプション
- -r テーブルファイルのパス：前計算したテーブルを読む場合に使う
- -m ：計算した対応関係を新たに記録する場合に使う(off推奨.default off)

前計算テーブルの作成
```
./MAX2sphere_precalc -m -o 出力ファイルのパス track0ファイルのパス track5ファイルのパス
```
## Installation

The MAX2sphere command line utility should build out of the box on Linux using the simple Makefile provided. The only external dependency is the standard jpeg library (libjpeg), the lib and include directories need to be on the gcc build path. The same applies to MacOS except Xcode and command line tools need to be installed.

```
$ git clone https://github.com/trek-view/MAX2sphere
$ make -f Makefile
$ @SYSTEM_PATH/MAX2sphere
```

## Usage

### Preparation

This script is designed to be used with frames. You will need to first convert a `.360` video to frames and then pass the two corresponding frames to the script ([2 frames as .360's use 2 video tracks](https://www.trekview.org/blog/2021/reverse-engineering-gopro-360-file-format-part-1/)).

You can use ffmpeg to split your `.360` video into frames (below at a rate of 1 FPS).

```
$ ffmpeg -i INPUT.360 -map 0:0 -r 1 -q:v 1 track0/img%d.jpg -map 0:5 -r 1 -q:v 1 track5/img%d.jpg
```

### Script

```
$ MAX2sphere [options] track0filename track5filename
```

Options:

* -w n sets the output image width, default: 5376 (for 5.6k)
* -a n sets antialiasing level, default = 2
* -o s specify the output filename, default is based on track0 name
* -d enable debug mode, default: off

#### Examples (MacOS)

##### Use a GoPro Max 3K video (width = 2272)

```
$ @SYSTEM_PATH/MAX2sphere -w 2272 testframes/track0_0010.jpg testframes/track5_0010.jpg
```

##### Use a custom output filename (name = davids-sphere)

```
$ @SYSTEM_PATH/MAX2sphere -o "output/davids-demo-sphere.jpg" testframes/track0_dirt.jpg testframes/track5_dirt.jpg
```

### Metadata

Note, the resulting image frames will not have any metadata -- this is not covered by the script.

[These docs explain what metadata should be added](https://guides.trekview.org/explorer/developer-docs/sequence-functions/process#videos-360s).

### Camera support

This script has currently been tested with the following cameras and modes:

* GoPro MAX
	* 360 Video (output .360)
		* 5.6K stitched (24FPS, 30FPS)
		* 3k stitched (60 FPS)
	* 360 TimeWarp (output .360)
		* (2x,5x,10x,15x,30x frame rate)

## Support

Join our Discord community and get in direct contact with the Trek View team, and the wider Trek View community.

[Join the Trek View Discord server](https://discord.gg/ZVk7h9hCfw).

## License

The code of this site is licensed under an [MIT License](/LICENSE).

