# LZW compression

```
:\> g++ lzw.cpp -o lzw
```

**Compression**
```
:\> .\lzw –c <lzw filename> <a list of files>
:\> .\lzw –c example.lzw a.txt b.txt c.txt

:\> .\lzw –cc <lzw filename> <a list of files>
:\> .\lzw –cc example.lzw a.txt b.txt c.txt

:\> .\lzw –CC <lzw filename> <one file name>
:\> .\lzw –cC example.lzw a.txt 
```
```
-c <=> -d
-cc <=> -dd
-CC <=> -DD
```
**note: enhanced LZW -CC only suport one file**

**Decompression**
```
:\> .\lzw –d <lzw filename>
:\> .\lzw –dd <lzw filename>
:\> .\lzw –DD <lzw filename>
```

command -c, -d, -C, -D, -CC, -DD.
1. -c, -d, basic implementation
2. -C, -D, map implementation
3. -CC, -DD, map with enhanced LZW

For the 1st implementation, which is -c and -d. I used a simple table implementation. Instead of storing the whole string, only the prefix code of character is stored. So, to retrieve the string, it will need to go through prefix code -> until -1. Which is initialized code. This may take a bit longer time, since simple table loop is O(n) and this will be O(nlogn), but this saves a lot of space. E.g. for a long string, abcdefghijk, we are storing all prefix string a, ab, abc, abcd, abcde,… This approach only store two CODE_SIZE code per node, which reduce space complexity from O(n^2) to O(n). 

For the 2nd implementation, -C, -D, I implement it using map. The map store <string, int> with the string as key, and a string array of 4096 for decoding. Since it used hash map, time to retrieve a string is simply O(1), and the space complexity is O(n^2) for a string, Since I stored the whole prefix string. If used previous method, it would take O(logn) and O(n) for time and space complexity respectively. Since retrieving the string will still need to go from bottom node to initialized node to get whole string.

For 3rd implementation, -CC, -DD, it is a hash map enhanced version, the code_size of each write code is variable from 9 to 12, according to n. (N< 511 then 9, N< 1023 then 10, etc, where N is map size). This reduce the size of lzw by about 7.5% at most during testing. Also, since the size of code_size is variable, so it is very difficult and almost impossible to know if the EOF is the byte read, so this command can only work for single file. 

For clearer view, I have made a table showing space and time complexity for string.

| |	1st implementation	| 2nd implementation | 3rd implementation |
| :---: | :---:  | :---: | :---: |
|Time complexity | O(nlogn) | O(1) |O(1) |
|Space complexity |	O(n) | O(n^2) | O(n^2) |

# DCT and IDCT

DCT is used before compression in run-length encoding in JPG compression. After Discrete Cosine Transformation, most image information/value will concentrate in top left part of image matrix. By applying Quantization Matrix to the image, other parts in image matrix become zero, making run-length encoding in compression efficient.

```
:\> g++ dct.cpp bmp.cpp -o dct
```
#### DCT
![image](https://user-images.githubusercontent.com/39010822/165595901-1f49c6d4-e6c3-4305-9812-9afc90008224.png)

**Use efficent version**
![image](https://user-images.githubusercontent.com/39010822/165596695-8ffd43ab-19e7-4406-b4a4-34fd3979d6a3.png)

#### IDCT

![image](https://user-images.githubusercontent.com/39010822/165596840-b5eae6f5-9f96-4369-809d-f15214476128.png)


```
:\> dct.exe <img_path> <apply_idct>
:\> dct.exe example.bmp 1
```
