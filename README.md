# MyHex Editor
MyHex是一个轻量级的Windows平台下二进制文件的16进制码查看器。

## 一句话用法
> myhex \<filename\>

**filename** 这里指的是需要被查看HEX码的文件的路径。相对路径和绝对路径均可。

## 杂项

附上字符串数字转换为int整型数字的代码。
```
int str2int(string s) {
    int ans;
    int negative=0;
    if(s[0]=='-') {
        negative=1;
    }
    for(int i=0;i<s.size();i++) {
        if(s[i]>='0' && s[i]<='9') {
            ans=ans*10+s[i]-'0';
        }
        else {
            return -1;
        }
    }
    if(negative=0) {
        return ans;
    }
    else {
        return -ans;
    }
}
```
