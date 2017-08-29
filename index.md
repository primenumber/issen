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
|#40|  +38| 0.56s|20.7M|
|#41|    0| 1.53s|51.0M|
|#42|   +6| 4.20s| 192M|
|#43|  -12| 4.57s| 135M|
|#44|  -14| 4.56s| 228M|
|#45|   +6|14.78s| 633M|
|#46|   -8|41.97s|2.17G|
|#47|   +4| 4.24s| 189M|
|#48|  +28|20.89s| 950M|
|#49|  +16|139.7s|6.78G|
|#50|  +10|127.2s|6.10G|
|#51|   +6|262.9s|12.0G|
|#52|    0|140.3s|7.29G|
|#53|   -2|386.8s|18.1G|
|#54|   -2| 1726s|91.9G|
|#55|    0| 2350s| 114G|
|#56|   +2|593.4s|22.8G|
|#57|  -10|706.9s|31.1G|
|#58|   +4| 1080s|45.7G|
|#59|  +64| 0.45s| 320k|
