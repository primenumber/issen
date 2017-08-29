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

Last Updated: 2017/08/29

|No.|Score| Time |Nodes|
|---|-----|------|-----|
|#40|  +38| 0.49s|19.0M|
|#41|    0| 1.45s|49.4M|
|#42|   +6| 3.49s| 158M|
|#43|  -12| 3.14s| 129M|
|#44|  -14| 4.62s| 222M|
|#45|   +6|12.70s| 599M|
|#46|   -8|43.40s|2.13G|
|#47|   +4| 4.15s| 177M|
|#48|  +28|18.63s| 943M|
|#49|  +16|134.5s|6.74G|
|#50|  +10|130.7s|6.08G|
|#51|   +6|239.0s|11.3G|
|#52|    0|146.4s|7.25G|
|#53|   -2|415.1s|17.9G|
|#54|   -2| 1468s|84.7G|
|#55|    0| 2404s| 113G|
|#56|   +2|559.3s|22.5G|
|#57|  -10|628.2s|28.3G|
|#58|   +4| 1057s|45.7G|
|#59|  +64| 0.43s| 283k|
