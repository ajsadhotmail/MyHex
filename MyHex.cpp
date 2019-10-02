#include<bits/stdc++.h>
using namespace std;
char str[105];
string c(string s)
{
	for(int i=0;i<s.size();i++)
	{
		if(s[i]>='a' && s[i]<='z')
			s[i]-=32;
	}
	return s;
}
int main(int argc, char* argv[])
{
	if(argc<2)
	{
		cout<<"Usage: re <filename>"<<endl;
		cout<<"This is a HEX Viewer"<<endl;
		return 0;
	}
	ifstream in;
	char s;
	int n=0;
	in.open(argv[1],ios::in | ios::binary);
	if(!in)
	{
		cout<<"File not found.\n";
		return 0;
	}
	cout<<"[Address] 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F";
	while(in.read((char*)&s,sizeof(s)))
	{
		if(n%16==0)
		{
			if((int)s<0)
				s=-s;
			cout<<endl;
			itoa((int)s,str,16);
			string tmp=str;
			printf("[%07x] ",n);
			if(tmp.size()==1)
				cout<<"0"<<c(tmp)<<" ";
			else if(tmp.size()==8)
				cout<<c(tmp.substr(6,tmp.size()))<<" ";
			else
				cout<<c(tmp)<<" ";
			n++;
		}
		else
		{
			itoa((int)s,str,16);
			string tmp=str;
			if(tmp.size()==1)
				cout<<"0"<<c(tmp)<<" ";
			else if(tmp.size()==8)
				cout<<c(tmp.substr(6,tmp.size()))<<" ";
			else
				cout<<c(tmp)<<" ";
			n++;
		}
	}
	in.close();
	cout<<"\nCount : "<<n/16<<" rows.";
	cout<<endl;
	return 0;
}
