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
|#40|  +38| 0.92s|22.3M|
|#41|    0| 2.37s|59.9M|
|#42|   +6| 3.67s|98.9M|
|#43|  -12| 7.29s| 153M|
|#44|  -14| 2.27s|46.9M|
|#45|   +6|27.20s|68.2M|
|#46|   -8|19.97s| 590M|
|#47|   +4| 5.97s| 155M|
|#48|  +28|38.14s|1.07G|
|#49|  +16|137.5s|3.28G|
|#50|  +10|126.0s|3.11G|
|#51|   +6|252.6s|7.14G|
|#52|    0|177.8s|4.89G|
|#53|   -2|767.6s|20.8G|
|#54|   -2| 1703s|52.0G|
|#55|    0| 3611s| 102G|
|#56|   +2|700.5s|17.7G|
|#57|  -10|593.2s|15.0G|
|#58|   +4| 1994s|58.1G|
|#59|  +64| 1.81s|36.5M|
