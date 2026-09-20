コンテナで make を走らせる
```
docker run --rm --platform linux/amd64 -v $HOME/dev/kcc:/kcc -w /kcc compilerbook make
```

コンテナでシェルを起動してインタラクティブに使う
```
docker run --rm --platform linux/amd64 -it -v $HOME/dev/kcc:/kcc compilerbook
```
