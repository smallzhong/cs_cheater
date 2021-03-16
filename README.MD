# CS辅助

+ 考虑到国内使用github下载比较缓慢，如果git clone无法成功下载代码的可以通过百度网盘下载本项目中的代码。

  链接：https://pan.baidu.com/s/1hrJ0rg9rrowfpAE8Zi3Hog 
  提取码：1111 

+ 另附本项目配套使用的cs游戏版本

  链接：https://pan.baidu.com/s/1lC8WfNqU6lVZ6yupNXXjFw 
  提取码：1111 

## 使用效果

+ [x] 锁血
+ [x] 飞天遁地
+ [x] 方框透
+ [ ] 无限手雷
+ [ ] iathook加速

+ ![image-20210316134928788](https://cdn.jsdelivr.net/gh/smallzhong/picgo-pic-bed/image-20210316134928788.png)
+ ![image-20210316140044616](https://cdn.jsdelivr.net/gh/smallzhong/picgo-pic-bed/image-20210316140044616.png)



## 注意事项

+ 本项目是cs起源的一个外挂程序，实现了锁血、方框透视的功能。由于我下载的这个cs不是官方版本，所以其各种数据的基址可能和其他版本不太一样。如果想要使用这个外挂请下载我使用的cs。
+ 锁血实现的方法是不断往存自己血量的位置写入一个很大的数字，以达到血量不减少的目的。血量基址见readme后附基址。
+ 飞天遁地实现的方法是通过强制修改z轴坐标让人物位置改变。人物xyz轴坐标基址见readme后附基址。
+ 方框透视是计算出在屏幕上显示的位置后通过rectangle函数画出矩形。这里使用了透明窗口，不会闪烁。
+ 我尝试通过iathook进行加速，但是总是hook不上getticicount、timegettime、query什么的那个函数，但是hook `Messagebox` 又能正确hook，挺迷惑的。我的做法是远程线程注入然后通过 `dll` 进行 `iathook` ，dll的代码放在 `dll2` 目录下。这个功能做了一天半还是没做出来，不知道问题在哪，搁置了，以后再做。

## 基址

+ 因为没有下到原版的cs1.6，网上查到的基址全都不能用，因此基址全部都是自己手动一个一个找出来的。花了很长时间。

+ 自身信息存放结构体基址

  ```
  pMyBase = server_css.dll+3B5D18
  ```

  自身血量存放位置

  ```
  血 = *pMyBase + 9c
  ```

  自身xyz坐标存放位置

  ```
  x = *pmybase + 0x2c4 - 8
  y = *pmybase + 0x2c4 - 4
  z = *pmybase + 0x2c4
  ```

  自身视角坐标存放位置

  ```
  上下仰角（向下为正向上为负） = engine.dll + 0x326EC0
  左右（-180~180） = engine.dll + 0x326ebc
  ```

+ 敌人信息存放结构体指针数组基址

  ```
  server_css.dll + 0x3D24E4
  ```

  每隔 `0x10` 个字节存放一个指向敌人结构体的指针。可以这样遍历

  ```cpp
  for (int i = 0; i < enemy_count; i ++ )
  {
      PVOID ptraddr = (PVOID)((PCHAR)g_server_css_base + 0x3D24E4 + i * 0x10);
  	ReadProcessMemory(g_hprocess, ptraddr, &buffer, 4, &dwread);
  }
  ```

  得到指向某个敌人结构体的指针之后，敌人位置

  ```
  x = *pEnemyBase + 0x2c4 - 8
  y = *pEnemyBase + 0x2c4 - 4
  z = *pEnemyBase + 0x2c4
  ```

  敌人血量

  ```
  血 = *pEnemyBase + 0x9c
  ```

  这里敌人死亡的时候血量为1，不知道为什么。因此做透视功能的时候想让敌人死了之后框框消失，我的做法是当血量为1时消失。因此有可能会出现敌人血量刚好为1但是没死的情况。按道理应该有一个数据用来存储这个人是否死亡，但是我一直没找到。如果您找到了可以给我提issue或者pr，万分感谢！

