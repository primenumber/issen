issen
=====

高速なOthelloAIの実装

## ビルドと実行

- C++14に対応したコンパイラと、 Boost C++ Librariesが必要
- Intel C++ Compiler以外でコンパイルできるかどうかは未確認
- SSE\*とpopcountに対応したCPUでないと動作しません

### 初期化

```
$ git submodule init
$ git submodule update
$ bundle install --path=vendor/bundle
$ autoheader
$ aclocal
$ automake --add-missing --copy
$ autoconf
$ ./configure
```

### コンパイル

    $ make

### 実行

- オセロサーバーに接続してプレイする

    $ bundle exec ruby server.rb

- [ffotest](http://www.radagast.se/othello/ffotest.html)

    $ ./issen --ffotest < input.pos

## TODO

- 実際に対戦ができるようにする
- 評価関数をまともなものに変更する
- AVXに移行する
