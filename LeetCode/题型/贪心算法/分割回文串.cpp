#include <string>
#include <vector>
using namespace std;

class Solution {
private:
    vector<vector<bool>>dp;
    vector<string>arr;
    vector<vector<string>>ans;
public:
    void dfs(string s,int index){
        if(index==s.size()){
            ans.push_back(arr);
        }
        for(int j=index;j<s.size();j++){
            if(dp[index][j]){
                arr.push_back(s.substr(index,j-index+1));
                dfs(s,j+1);
                arr.pop_back();
            }
        }

    }

    vector<vector<string>> partition(string s) {
        int n=s.size();

        dp.assign(n,vector<bool>(n,true));

        for(int i=n-1;i>=0;i--){
            for(int j=i+1;j<n;j++){
                dp[i][j]=(s[i]==s[j])&&dp[i+1][j-1];
            }
        }

        dfs(s,0);

        return ans;
    }
};