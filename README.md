issen
=====

高速なOthelloAIの実装

This project isn't maintained anymore, moved to [issen-rs](https://github.com/primenumber/issen-rs).

## ビルドと実行

- C++14に対応したコンパイラと、 Boost C++ Librariesが必要
- AVX2とBMIに対応したCPUでないと動作しません(Haswell以降)

### 初期化

```
$ cmake .
```

### コンパイル

    $ make

### 実行

[ffotest](http://www.radagast.se/othello/ffotest.html)

    $ VAL_PATH=path/to/val src/issen --ffotest < input.pos
    
ベンチ結果は[こちら](http://primenumber.github.io/issen/)

## Base81

オセロの盤面を4マスずつに分け、3^4 = 81通りをASCII文字に対応させることで盤面を16バイトのASCII文字列で表現できる。
空き=0, 黒石(or 自石)=1, 白石(or 相手石)=2として、

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
