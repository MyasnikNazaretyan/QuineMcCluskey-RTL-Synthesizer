#pragma once
#include <cstdint>
#include <vector>
#include <string>
using namespace std;
class Implicant{
private:
     mutable uint32_t value{0};
     uint32_t mask{0};
    vector<uint32_t> minterms;
    mutable bool combined{false};
public:
Implicant()=default;
Implicant(uint32_t val):value(val){minterms.push_back(val);}
bool& isCompined() const {return combined;}
const vector<uint32_t>& isminterms() const {return minterms;}
bool operator ==(const Implicant& other) const{
    return (value==other.value) && (mask==other.mask);
}    
string toBoolean(int numVars) const{
    string result;
    char varName;
    string str=toString(numVars);
    for(int i=0;i<str.size();i++){
        varName='A'+i;
        if(str[i]=='1') result.push_back(varName);
        else if(str[i]=='0') {result.push_back(varName),result.push_back('`'); }
    }
     return result;
}
int CountOnes() const{  //std:: popcount(value) հրահանգով կարելի է հաշվել O(1) ժամանակում;
        int count=0;
        uint32_t temp=value;
        while(temp>0){
            temp=temp & (temp-1);
            count++;
        }
        return count;
    }
    Implicant Combine(const Implicant& other) const{
        Implicant result;
        result.value=this->value & other.value;
        result.mask=this->mask | other.mask | (this->value ^ other.value);
        result.minterms.reserve(this->minterms.size()+other.minterms.size());
        result.minterms.insert(result.minterms.end(),other.minterms.begin(),other.minterms.end());
        result.minterms.insert(result.minterms.end(), this->minterms.begin(), this->minterms.end());
        this->combined=true;
        other.combined=true;
        return result;
    }
    bool canCombine(const Implicant& other) const{
       if((mask ^ other.mask)!=0) return false;
        uint32_t diff=value^other.value;
        return ((diff!=0) && (diff &(diff-1))==0);
    }
    string toString(int numvars) const{
        string result;
        for(int i=numvars-1;i>=0;i--){
            if(((1U<<i) & mask)!=0) result.push_back('-');
            else if(((1U<<i) & value)!=0)  result.push_back('1');
            else result.push_back('0');
        }
        return result;
    }
};