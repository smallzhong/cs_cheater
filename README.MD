# CS辅助

- [x] 锁血
- [x] 飞天遁地
- [x] 方框透
- [x] 方框不闪



## 基址

+ 血：

  ```
  server_css.dll+3B5D18
  偏移+9c
  ```

  ![image-20210308202155710](https://cdn.jsdelivr.net/gh/smallzhong/picgo-pic-bed/image-20210308202155710.png)

+ 自己z `server_css.dll+3B5D18` . +2c4

+ ![image-20210309161200329](https://cdn.jsdelivr.net/gh/smallzhong/picgo-pic-bed/image-20210309161200329.png)

+ 敌人z基址

  ```c
  server_css.dll+3D24E4
  ```

+ 自己的XYZ轴这个好像不太对劲，强制修改有时候能飞天，有时候不能

+ 人物结构体 `+8b * 4 = 22c` 得到其是什么阵营的

  