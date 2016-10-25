//
//  main.cpp
//  HomophonicBreaker
//
//  Created by Cryptosquad
//
//

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <algorithm>
#include <math.h>
#include <numeric>
#include <sstream>
#include <time.h>
#include <iomanip>
using namespace std;


//constants

map<char,int> LETTERS_FREQUENCY = { {'a',8} ,{'b',1},{'c',3},{'d',4}, {'e',13} , {'f',2} ,{'g',2} ,{'h',6} , {'i',7},{'j',1},{'k',1},{'l',4},{'m',2},{'n',7},{'o',9},{'p',2},{'q',1},{'r',6},{'s',6},{'t',9},{'u',3} ,{'v',1},{'w',2},{'x',1},{'y',2},{'z',1} };
vector<vector<string>> PLAIN_TEXT(10);
vector<string> ENGLISH_WORDS;
map<string,float> ENGLISH_QUADGRAMS;
map<string,float> ENGLISH_TRIGRAMS;

#define KEYTRIES 200000 // # of keys to try before returning the best
#define ACCURATE 0.95 //return the key
#define THRESHOLD 100 //# of fails needed to move on to rankExc2

//constants population
void readEnglishQuads();
void readEnglishTris();
void readEnglishDic();
void readPlainText();


class key{
public:
    key();//generates a random currentKey
    key(const vector<char>& a);//constructs using a ready mapping vector
    key mutate(int factor = 2);//mutates the key by factor of factor
    void printKey() const;
    vector<string> decrypt(const vector<vector<int>>& cipher);//takes a collection of encoded words and returns the deciphred text according to currentKey
    float rankEXC1(const vector<string>& words); //given a deciphered text, returns a rank for exc1 part
    float rankEXC2(const vector<string>& words); //given a deciphered text, returns a rank for exc2 part
    void printDeciphered(vector<string> &words);
    void printCipherForTesting();//NOT EFFICIENT, JUST FOR TESTING
    
    float rankDic(const vector<string>& deciphered);
    //returns 0 if there's no match to the given dictionaries
    //returns 0-1 not inclusive if there is a ranking to one of the options
    //returns 1-10 indicating if the right plaintext was found
    
    float rankAbc(const vector<string> &deciphered);//ranks the key based on 4 letter words
    float rankAbc2(const vector<string> &deciphered);//ranks the key based on 4 letter words

private:
    vector<char> currentKey;//maps each index from 0-102 to a character in the abc
};

//auxilary functions

size_t getLastThreeSizes(const vector<string>& deciphered);//returns the sum of the sizes of the last three words in the ciphertext

void organizeData(const string& input, vector<vector<int>>& data);
//void organizeData(ifstream& input, vector<vector<int>>& data);
key findMax(const vector<vector<int>>& data, int& blocknum);

//function that runs ABC and returns an index
// if ABC returns 0 10 times in a row, move on.


int main1() {
    
    //constants population
    readPlainText();
    readEnglishDic();
    readEnglishTris();
    readEnglishQuads();
    
    key randomKeyForTesting;
    //randomKeyForTesting.printCipherForTesting();
    
    ifstream file("testInput.txt");
    string input;
    //cout << "Enter the ciphertext: " << endl;
    if (getline(file, input)) {  //get input, only works if there is a ' ' at the end
        vector<vector<int>> data;
        organizeData(input, data); //put input into data
        //cout << "last three letter sizes: " << data[data.size()-1].size() + data[data.size()-2].size() + data[data.size()-3].size() << endl;
        //cout <<" data size is " << data.size() << endl;
        /*
        for(int i=0; i<data.size(); i++){
            for(int j=0; j<data[i].size(); j++){
                cout << data[i][j] <<",";
            }
            cout <<" ";
        }
         */
		cout << "Program started trying to find the plaintext, please wait." << endl;
        int blocknum = 0;
        key bestkey = findMax(data, blocknum);  //call findMax which will return the best key no matter which test
        if(blocknum > 0){
            cout << "The estimated plaintext is the plaintext num: " << blocknum << endl;
            for(const string& word : PLAIN_TEXT[blocknum-1]){
                cout << word << " ";
            }
            cout << endl;
        }
        
        else{
            vector<string> output = bestkey.decrypt(data); //decrypt input using the best key
            //Should we make a function to make 90% matching words to 100%?
            cout << "The estimated plaintext is: ";
            for (int i = 0; i < output.size(); i++) {
                cout << output[i] << " ";
            }
            cout << endl;
        }
        
    }
    return 0;
}


void organizeData(ifstream& input, vector<vector<int>>& data) {
    int cipher;
    char sign;
    vector<int> temp;
    while (input >> noskipws >> cipher >>sign) {
        //cout << cipher <<"sign:" << "aa" << sign << "aa"<< endl;
        if (sign == ',') {
            temp.push_back(cipher);
        }
        else {
            cout <<"WHITESPACE" << endl;
            temp.push_back(cipher);
            data.push_back(temp);
            temp.clear();
        }
    }
    if (temp.size() != 0){
        temp.push_back(cipher);
        data.push_back(temp);
        temp.clear();
    }
}

void organizeData(const string& input, vector<vector<int>>& data) {
    int cipher;
    char sign;
    stringstream ss(input);
    vector<int> temp;
    //cout << "DATA: " << endl;
    while (ss >> noskipws >> cipher >>sign) {
        //cout << cipher <<"sign:" <<sign<< endl;
        //cout<<cipher<<sign;
        if (sign == ',') {
            temp.push_back(cipher);
        }
        else  {
            temp.push_back(cipher);
            data.push_back(temp);
            temp.clear();
        }
    }
    if (temp.size() != 0){
        temp.push_back(cipher);
        data.push_back(temp);
        temp.clear();
    }
}


key findMax(const vector<vector<int>>& data, int& blocknum) {
    float bestRank = 0;
    key temp;
    key bestKey = temp;
    int factor = 100;
    int count = 0;
    cout << "trying EXC1..." << endl;
    for (int i = 0; i < KEYTRIES; i++) {
        //cout << "trying a key" << endl;
        temp = bestKey.mutate(factor);
        //cout << "mutating done" << endl;
        vector<string> result = temp.decrypt(data);
        //cout << "decrypted" << endl;
        float ranking = temp.rankEXC1(result);
        //cout << setprecision(9) <<"ranking for EXC1 is: " << ranking << endl;
        
        if (ranking >=1){
            blocknum = ranking;
            bestKey = temp;
            break;
        }
        if (ranking > bestRank) {
            bestRank = ranking;
            bestKey = temp;
        }
        if (ranking > 0.05 ) factor=20;
        if (ranking > 0.1 ) factor=8;
        if(ranking > 0.2) factor = 2;
        
        if (ranking == 0) {
            count++;
        }
        
        if (count > THRESHOLD) {
            cout << "NOT EXCERCISE 1" << endl;
            break;
        }
    }
    factor=100;
    if (count > THRESHOLD) {
        cout << "TRYING EXC2" << endl;
        for (int i = 0; i < KEYTRIES; i++) {
            temp = bestKey.mutate(factor);
            vector<string> result = temp.decrypt(data);
            float ranking = temp.rankEXC2(result);
            //cout << setprecision(9) <<"ranking for EXC2 is: " << ranking << endl;
            if (ranking > 0.0001 ) factor=60;
            if (ranking > 0.0003 ) factor=20;
            if (ranking > 0.0004 ) factor=15;
            if (ranking > 0.0005 ) factor=10;
            if (ranking > 0.001 ) factor=4;
            if (ranking > 0.0015 ) factor=3;
            if(ranking > 0.0017) factor = 2;

            //if(ranking > 0.0045) factor = 12;
            //if(ranking > 0.005) factor = 11;
            if (ranking > bestRank) {
                bestRank = ranking;
                bestKey = temp;
            }
        }
    }
    
    return bestKey;
}
 
 
 




int main2(){
    
    key k;
    k.printKey();
    key k2 = k.mutate(10);
    k2.printKey();
    k2.printCipherForTesting();
    return 0 ;
    
}



//constant population
void readEnglishTris(){
    ifstream file("english_trigrams.txt");
    if(!file) {
        cerr << "error openning english_trigrams.txt" << endl;
        exit(1);
    }
    string next_quad;
    float next_ratio;
    long sum = 0;
    while(file>>next_quad){
        std::transform(next_quad.begin(), next_quad.end(), next_quad.begin(), ::tolower);
        //cout << next_quad << endl;
        ENGLISH_TRIGRAMS[next_quad] =0;
        file>>next_ratio;
        sum+=next_ratio;
        ENGLISH_TRIGRAMS[next_quad] = next_ratio / 6020781834240.00000000; //the quads count divided by the number of all quads appearances gives the quad's probability
    }
    /*
     for (map<string,float>::iterator i = ENGLISH_TRIGRAMS.begin(); i!= ENGLISH_TRIGRAMS.end(); i++){
     cout << i->first << i->second << endl;
     }*/
    //for debugging
    //cout << "SUM: " << sum << endl;
    //cout << ENGLISH_TRIGRAMS.size() << endl;
}
void readEnglishQuads(){
    ifstream file("english_quadgrams.txt");
    if(!file) {
        cerr << "error openning english_quadgrams.txt" << endl;
        exit(1);
    }
    string next_quad;
    float next_ratio;
    while(file>>next_quad){
        std::transform(next_quad.begin(), next_quad.end(), next_quad.begin(), ::tolower);
        //cout << next_quad << endl;
        ENGLISH_QUADGRAMS[next_quad] =0;
        file>>next_ratio;
        ENGLISH_QUADGRAMS[next_quad] = next_ratio / 42478629.00000000; //the quads count divided by the number of all quads appearances gives the quad's probability
    }
    /*
    for (map<string,float>::iterator i = ENGLISH_QUADGRAMS.begin(); i!= ENGLISH_QUADGRAMS.end(); i++){
        cout << i->first << i->second << endl;
    }*/
    //for debugging
    //cout << ENGLISH_QUADGRAMS.size() << endl;
}

void readEnglishDic(){
    ifstream file("english_words.txt");
    if(!file) {
        cerr << "error openning plainttext_dictionary.txt" << endl;
        exit(1);
    }
    string next_word;
    while(file>>next_word){
        ENGLISH_WORDS.push_back(next_word);
    }
}
void readPlainText(){
    ifstream file("plaintext_dictionary.txt");
    
    if(!file) {
        cerr << "error openning plainttext_dictionary.txt" << endl;
        exit(1);
    }
    int i = 0;
    string next_word;
    while(file >> next_word){
        PLAIN_TEXT[i].push_back(next_word);
        if(next_word == "fluke" || next_word == "spenc" || next_word == "unventilat" || next_word == "exacted" || next_word == "p" || next_word == "n" || next_word == "sto" || next_word == "dat" || next_word == "apotheosi" || next_word == "cloudiest"){
            i++;
        }
        
    }
    
    //for debugging
    /*
    for(int j = 0 ; j<PLAIN_TEXT.size(); j++){
        cout << "printing plaintext #" << j+1 << endl;
        for(int k = 0 ; k<PLAIN_TEXT[j].size(); k++){
            cout << PLAIN_TEXT[j][k] << " ";
        }
        cout << "sizes" << endl;
        for(int k = 0 ; k<PLAIN_TEXT[j].size(); k++){
            cout << PLAIN_TEXT[j][k].size() << " ";
        }
        cout << endl << endl;
    }
     */
    
}


//key class functions
key::key(): currentKey(103){
    srand(time(NULL));
    vector <int> ops;
    for(int i=0;i<103; i++){
        ops.push_back(i);
    }
    vector<char> ltrs = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    random_shuffle(ltrs.begin(), ltrs.end());
    int randIndex;
    char randLtr;
    map<char,int> frqCopy = LETTERS_FREQUENCY;
    int counter = 0;
    while(counter < 103){
        randLtr = ltrs[rand()%ltrs.size()];
        randIndex = rand()%ops.size();
        if(frqCopy[randLtr] > 0){
            currentKey[ops[randIndex]] = randLtr;
            
            
            //cout << "inserted " << ops[randIndex] << " into "<< randLtr << " and now we can only enter " << frqCopy[randLtr]-1 << endl;
            ops[randIndex] = ops[ops.size()-1];
            frqCopy[randLtr]--;
            ops.pop_back();
            counter++;
        }
    }
};

key::key(const vector<char>& a): currentKey(a){};

key key::mutate(int factor){
    vector<int> rands;
    vector <int> ops;//vector of options from 0-102
    int randI;
    vector<char> copyKey = currentKey;
    for(int i=0;i<103; i++){
        ops.push_back(i);
    }
    for(int i = 0; i<factor ; i++){
        randI = rand()%ops.size();
        rands.push_back(ops[randI]);
        ops[randI] = ops[ops.size()-1];
        ops.pop_back();
    }
    /*
    cout << "rands " << endl;
    for(int i=0;i<rands.size(); i++){
        cout << rands[i] << " ";
    }
     */
    ////
    //cout << " swapping.." << endl;
    
    ////

    for (int k = 0 ; k<factor-1; k++){
        //cout << "letter " << copyKey[rands[k]] << endl;
        if(k == factor -1 ){
            swap(copyKey[rands[k]],copyKey[rands[0]]);
            //cout << "swapping " << copyKey[rands[k]] << " and " << copyKey[rands[0]]<< endl;
        }
        else{
            swap(copyKey[rands[k]],copyKey[rands[k+1]]);
            swap(rands[k],rands[k+1]);
            //cout << "swapping " << copyKey[rands[k]] << " and " << copyKey[rands[k+1]]<< endl;
        }
    }
    //cout << "finised swapping" << endl;
    return key(copyKey);
}

void key::printKey() const{
    int counter = 0;
    vector<char> ltrs = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','t','s','u','v','w','x','y','z'};
    
    for(int i=0;i<ltrs.size();i++){
        cout << ltrs[i] << ": ";
        for (int k=0; k<currentKey.size(); k++){
            if (currentKey[k] == ltrs[i]){
                cout << k << ", ";
                counter++;
            }
        }
        cout << endl;
    }
    cout << counter << " letters were printed" << endl;
    
}

vector<string> key::decrypt(const vector<vector<int>>& cipher){
    vector<string> decrypted;
    string newWord;
    for(const vector<int>& word: cipher){
        newWord = "";
        for(int decLtr : word){
            newWord += currentKey[decLtr];
        }
        decrypted.push_back(newWord);
    }
    return decrypted;
}

float key::rankEXC1(const vector<string>& words){
    return rankDic(words);
}
float key::rankEXC2(const vector<string>& words){
    return rankAbc2(words);
}

void key::printDeciphered(vector<string> &words){
    for (size_t i = 0; i < words.size(); i++) {
        cout << words[i] << " ";
    }
}

void key::printCipherForTesting(){//NOT EFFICIENT, JUST FOR TESTING
    srand(time(NULL));
    ifstream file("testCipher.txt");
    if(!file) {
        cerr << "error openning testCipher.txt" << endl;
        exit(1);
    }
    string word;
    while(file>>word){
        for (int j=0; j<word.size(); j++){
            char c = word[j];
            int frequency = LETTERS_FREQUENCY[c];
            int goal = rand() % frequency +1;
            int counter = 1;
            for(int i=0;i<currentKey.size(); i++){
                if(currentKey[i] == c && counter++ == goal){
                    cout << i;///randomize to not pick the first one
                    if(j != word.size()-1) cout <<",";
                }
            }
        }
        cout << " ";
    }
    
    //for debugging
    //cout << ENGLISH_QUADGRAMS.size() << endl;
}

float key::rankAbc(const vector<string>& deciphered){//consider more cases (3 letter words)
    vector<float> likelyhoods;
    float likleyhood = 0;
    for(int i=0;i<deciphered.size(); i++){
        likleyhood = 0;
        if(deciphered[i].size() >= 4){
            for(int j = 0; j<deciphered[i].size()-3; j++){
                    likleyhood += ENGLISH_QUADGRAMS[deciphered[i].substr(j,j+5)];
                    //cout << " word: " << deciphered[i] << " quad: " << deciphered[i].substr(j,j+5) << " ratio " <<ENGLISH_QUADGRAMS[deciphered[i].substr(j,j+5)] << " liklyhood " << likleyhood<< endl;
                
            }
            likelyhoods.push_back(likleyhood/(deciphered[i].size()-3));
        }
    }
    float sum=0;
    for (int i=0; i<likelyhoods.size(); i++){
        sum+=likelyhoods[i];
    }
    return sum/ likelyhoods.size();
};


float key::rankAbc2(const vector<string>& deciphered){//consider more cases (3 letter words)
    vector<float> likelyhoods;
    float likleyhood = 0;
    for(int i=0;i<deciphered.size(); i++){
        likleyhood = 0;
        if(deciphered[i].size() >= 3){
            for(int j = 0; j<deciphered[i].size()-2; j++){
                likleyhood += ENGLISH_TRIGRAMS[deciphered[i].substr(j,j+4)];
                //cout << " word: " << deciphered[i] << " quad: " << deciphered[i].substr(j,j+5) << " ratio " <<ENGLISH_TRIGRAMS[deciphered[i].substr(j,j+5)] << " liklyhood " << likleyhood<< endl;
                
            }
            likelyhoods.push_back(likleyhood/(deciphered[i].size()-2));
        }
    }
    float sum=0;
    for (int i=0; i<likelyhoods.size(); i++){
        sum+=likelyhoods[i];
    }
    return sum/ likelyhoods.size();
};


float key::rankDic(const vector<string> &deciphered){
    //returns 0 if there's no match to the given dictionaries
    //returns 0-1 not inclusive if there is a ranking to one of the options
    //returns 1-10 indicating if the right plaintext was found
    
    size_t lastThreeSizes = getLastThreeSizes(deciphered);
    //cout << "got last three sizes: " << lastThreeSizes << endl;
    vector<int> options;
    if(lastThreeSizes == 27){
        options = {1,2};
    }
    else if(lastThreeSizes == 32){
        options = {3};
    }
    else if(lastThreeSizes == 29){
        options = {4};
    }
    else if(lastThreeSizes == 19){
        options = {5,6};
    }
    else if(lastThreeSizes == 31){
        options = {7,8};
    }
    else if(lastThreeSizes == 24){
        options = {9,10};
    }
    else return 0;
    //cout << "found matches" << endl;
    vector<int> matches_count(10,0);
    int char_counter = 0;
    for(int i=0;i<deciphered.size(); i++){//for each word in the deciphered text
        
        for(int& each:options){
            //cout << "comparing '"<<PLAIN_TEXT[each-1][i] <<"' to " << deciphered[i] << endl;
            if(each == -1) continue;
            if(PLAIN_TEXT[each-1].size() < i) {continue; each = -1;}
            if(deciphered[i].size() != PLAIN_TEXT[each][i].size()) {continue; each = -1;}
            
            for(int j = 0; j<deciphered[i].size(); j++){//for each character
                char_counter++;
                if(PLAIN_TEXT[each-1][i][j] == deciphered[i][j])
                {
                    matches_count[each-1]++; //cout<<"hit!"<<endl;
                    //cout << matches_count[each-1] << endl;
                }
                if(matches_count[each-1] > 350){//if a certain ciphertext is mathing well return that number
                    return each;
                }
            }
        
        }
    }
    //by this point the matches vector should be full of zeros if there is no real match
    int maxC = 0;
    for(int i=0; i<matches_count.size(); i++){
        if(matches_count[i] > maxC) maxC = matches_count[i];
    }
    //cout << "returning.." <<maxC<< endl;
    return (maxC/500.000)-0.001;
}

size_t getLastThreeSizes(const vector<string>& deciphered){
    size_t lastIndex = deciphered.size()-1;
    //cout << deciphered[lastIndex].size() << " "<<deciphered[lastIndex-1].size() << " "<<deciphered[lastIndex-2].size() << endl;
    //cout <<deciphered[lastIndex] << " "<<deciphered[lastIndex-1] << " "<<deciphered[lastIndex-2];
    return deciphered[lastIndex].size() + deciphered[lastIndex-1].size() + deciphered[lastIndex-2].size();
};


int main(){
    main1();
    return 0;
}
