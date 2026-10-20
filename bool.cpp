#include<iostream>
#include <cstdint>
#include <string>
#include<vector>
#include <algorithm>
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
class QuineMcCluskey {     
    public:
    vector<Implicant> PrimeImplicant(const vector<uint32_t>& minterms,int numVars){
    vector<vector<Implicant>> CurrentGroups(numVars+1);
    vector<Implicant> primeImplicants;
    for(int i=0;i<minterms.size();i++){
        Implicant im(minterms[i]);
        CurrentGroups[im.CountOnes()].push_back(im);
    }
    while(true){
        vector<vector<Implicant>> NextGroups(numVars+1);
        bool anyCombined= false;
        
        for(int i=0;i<CurrentGroups.size()-1;i++){
            for(int j=0;j<CurrentGroups[i].size();j++){
                for(int k=0;k<CurrentGroups[i+1].size();k++){
                     Implicant combinedImp;
                    if(CurrentGroups[i][j].canCombine(CurrentGroups[i+1][k])){
                        combinedImp=CurrentGroups[i][j].Combine(CurrentGroups[i+1][k]);
                        anyCombined=true;
                    
                    int ones=combinedImp.CountOnes();
                    bool exists=true;
                    for(const auto& existingImp: NextGroups[ones] ){
                        if(combinedImp==existingImp) {exists=false; break;}
                    }
                    if(exists) NextGroups[ones].push_back(combinedImp);
                   }
                }
            }
          
        }
        for(int i=0;i<CurrentGroups.size();i++){
            for(int j=0;j<CurrentGroups[i].size();j++){
                 if(!CurrentGroups[i][j].isCompined()) primeImplicants.push_back(CurrentGroups[i][j]);
            }
        }
        if(!anyCombined) break;
         else  CurrentGroups=std::move(NextGroups);
    }
   return primeImplicants;
}
vector<Implicant> getMinimalCover(const vector<Implicant>& primeImplicants,const vector<uint32_t>& minterms){
    vector<Implicant> Essential;
    for(int i=0;i<primeImplicants.size();i++){
        bool isEssential=false;
        const auto& myMinterms=primeImplicants[i].isminterms();
        for(int j=0;j<myMinterms.size();j++){
            bool foundInOther=false;
           for(int k=0;k<primeImplicants.size();k++){
            if(i==k) continue;
            const auto& otherMinterms=primeImplicants[k].isminterms();
            if(find(otherMinterms.begin(),otherMinterms.end(),myMinterms[j])!=otherMinterms.end()){
                foundInOther=true;
                break;
             }
            }
            if(!foundInOther){
                isEssential=true;
                break;
            }
         }
    if(isEssential)
     Essential.push_back(primeImplicants[i]);
    }
    vector<uint32_t> uncovered;
    for(uint32_t m:minterms){
        bool isCoverd=false;
        for(const auto& ess :Essential){
            const auto& em=ess.isminterms();
            if(find(em.begin(),em.end(),m)!=em.end())
            {
                isCoverd=true;
                break;
            }
        }
        if(!isCoverd) uncovered.push_back(m);
    }
    while(!uncovered.empty()){
        int bestIdx = -1;
        int maxCount = -1;
       for(int i=0;i<primeImplicants.size();i++){
        int count=0;
        const auto& p=primeImplicants[i].isminterms();
         for(const auto& u: uncovered){
            if(find(p.begin(),p.end(),u)!=p.end())
                count++;
         }
         if(count>maxCount){
            maxCount=count;
            bestIdx=i;
         }
       }
    Essential.push_back(primeImplicants[bestIdx]);
    const auto& bestMinterms=primeImplicants[bestIdx].isminterms();
    for(int i=uncovered.size()-1;i>=0;i--){
       if(find(bestMinterms.begin(),bestMinterms.end(),uncovered[i])!=bestMinterms.end())
       uncovered.erase(uncovered.begin()+i);
    }
}
    return Essential;
}
void generateVerilog(const vector<Implicant>& minimalCover, int numVars) const {
    cout << "\n// Generated Synthesizable Verilog RTL\n";
    cout << "module minimized_logic(\n    input ";
    for (int i = 0; i < numVars; ++i) {
        cout << (char)('A' + i) << (i + 1 < numVars ? ", " : ",\n");
    }
    cout << "    output F\n);\n";
    cout << "    assign F = ";
    for (size_t i = 0; i < minimalCover.size(); ++i) {
        string str = minimalCover[i].toString(numVars);
        cout << "(";
        bool firstLiteral = true;
        for (int j = 0; j < numVars; ++j) {
            if (str[j] == '-') continue;
            if (!firstLiteral) cout << " & ";
            if (str[j] == '0') cout << "~" << (char)('A' + j);
             else cout << (char)('A' + j);
             firstLiteral = false;
        }
        cout << ")";
        if (i +1 < minimalCover.size()) cout << " | ";
    }
    cout << ";\nendmodule\n";
}
};
int main() {
    QuineMcCluskey qm;
    vector<uint32_t> minterms = {0, 1, 2, 5, 6, 7, 8, 9, 10, 14};
    int numVars = 4;

    vector<Implicant> primes = qm.PrimeImplicant(minterms, numVars);
    vector<Implicant> Essential;
    Essential=qm.getMinimalCover(primes,minterms);
   // cout << "Prime Implicants:" << endl;
  //  for (const auto& p : primes) {
  //      cout << p.toString(numVars) << endl;
 //   }
 cout << "\nMinimized Boolean Expression:\nF = ";
    int counter=0;
   for(const auto& p: Essential){
    cout<<p.toBoolean(numVars);
    if(counter<Essential.size()-1) cout<<'+'<<" ";
    counter++;
}
qm.generateVerilog(Essential,numVars);
    return 0;
}