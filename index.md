issen
=====

高速なOthelloAIの実装

## ビルドと実行

- C++14に対応したコンパイラと、 Boost C++ Librariesが必要
- AVX2, BMIに対応したCPUでないと動作しません(Haswell以降)

### 初期化

```
$ git submodule init
$ git submodule update
$ cmake .
```

### コンパイル

    $ make
    
### 評価関数テーブルディレクトリの指定

    $ export VAL_PATH=/path/to/value/table/dir

### 実行

[ffotest](http://www.radagast.se/othello/ffotest.html)

    $ ./issen --ffotest < input.pos

## TODO

- さらなる高速化

# [FFO Benchmark Test](http://www.radagast.se/othello/ffotest.html)

* CPU: Intel Core i7-6700K @ 4.0GHz TB 4.2GHz
* Memory: 64GB
* OS: Arch Linux (Linux 4.12.6-1)
* C++ Compiler: GCC 7.1.1

Last Updated: 2017/08/26

|No.|Score| Time |Nodes|
|---|-----|------|-----|
|#40|  +38| 0.74s|28.1M|
|#41|    0| 1.82s|64.0M|
|#42|   +6| 5.69s| 298M|
|#43|  -12| 4.54s| 171M|
|#44|  -14| 2.99s| 135M|
|#45|   +6|17.84s| 818M|
|#46|   -8|43.34s|2.14G|
|#47|   +4| 4.42s| 174M|
|#48|  +28|24.13s|1.09G|
|#49|  +16|107.5s|4.98G|
|#50|  +10|95.41s|4.22G|
|#51|   +6|284.5s|13.3G|
|#52|    0|165.9s|7.86G|
|#53|   -2|532.1s|21.3G|
|#54|   -2| 1928s|94.7G|
|#55|    0| 2475s| 110G|
|#56|   +2|552.0s|20.3G|
|#57|  -10|416.6s|15.2G|
|#58|   +4| 1558s|59.3G|
|#59|  +64| 1.82s|36.6M|
