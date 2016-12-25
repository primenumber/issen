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
* OS: Arch Linux (Linux 4.8.6-1)
* C++ Compiler: GCC 6.2.1

Last Updated: 2016/11/07

|No.|Score| Time |Nodes|
|---|-----|------|-----|
|#40|  +38| 0.89s|17.0M|
|#41|    0| 1.79s|39.3M|
|#42|   +6| 1.93s|43.7M|
|#43|  -12| 5.83s| 124M|
|#44|  -14| 2.91s|68.6M|
|#45|   +6|40.15s|1.12G|
|#46|   -8|24.02s| 757M|
|#47|   +4| 5.96s| 165M|
|#48|  +28|54.59s|1.79G|
|#49|  +16|57.47s|1.83G|
|#50|  +10|395.7s|12.1G|
|#51|   +6|222.0s|6.72G|
|#52|    0|150.8s|4.58G|
|#53|   -2| 1317s|41.6G|
|#54|   -2| 2103s|67.3G|
|#55|    0| 7627s| 236G|
|#56|   +2|639.6s|19.2G|
|#57|  -10|681.7s|20.7G|
|#58|   +4| 2625s|81.3G|
|#59|  +64| 1.80s|48.3M|
