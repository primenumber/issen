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

## Base81

オセロの盤面を4マスずつに分け、3^4 = 81通りをASCII文字に対応させることで盤面を16バイトのASCII文字列で表現できる。

` code = 32 * a[3] + 9 * a[2] + 3 * a[1] + a[0] + 33`

盤面と文字との対応

```
 |ABCDEFGH
-+--------
1|[01][02]
2|[03][04]
3|[05][06]
4|[07][08]
5|[09][10]
6|[11][12]
7|[13][14]
8|[15][16]
```
