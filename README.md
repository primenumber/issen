issen
=====

高速なOthelloAIの実装

## ビルドと実行

- C++14に対応したコンパイラと、 Boost C++ Librariesが必要
- AVX2とBMIに対応したCPUでないと動作しません(Haswell以降)

### 初期化

```
$ git submodule init
$ git submodule update
$ cmake .
```

### コンパイル

    $ make

### 実行

[ffotest](http://www.radagast.se/othello/ffotest.html)

    $ ./issen --ffotest < input.pos

## TODO

- 実際に対戦ができるようにする
