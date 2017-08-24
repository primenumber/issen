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

Last Updated: 2017/08/23

|No.|Score| Time |Nodes|
|---|-----|------|-----|
|#40|  +38| 0.96s|19.5M|
|#41|    0| 2.64s|68.8M|
|#42|   +6| 1.80s|42.4M|
|#43|  -12| 2.18s|45.4M|
|#44|  -14| 3.01s|72.9M|
|#45|   +6|27.15s| 712M|
|#46|   -8|31.11s| 942M|
|#47|   +4| 6.65s| 179M|
|#48|  +28|38.85s|1.15G|
|#49|  +16|79.84s|2.39G|
|#50|  +10|126.0s|3.11G|
|#51|   +6|234.8s|6.84G|
|#52|    0|198.3s|5.75G|
|#53|   -2| 2327s|72.5G|
|#54|   -2| 1805s|54.4G|
|#55|    0| 5531s| 164G|
|#56|   +2|746.7s|21.8G|
|#57|  -10| 1175s|35.3G|
|#58|   +4| 3303s|99.1G|
|#59|  +64| 0.80s|16.3M|
