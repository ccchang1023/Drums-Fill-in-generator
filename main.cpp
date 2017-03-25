#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<cstdlib>
#include<cstring>
#include<ctime>
#include<vector>
#include<algorithm>
#include<climits>
#define INSTRUMENT_NUM 9

using namespace std;
vector<string> file_content;
vector<int> bd_pattern[4];  // bass drum pattern
vector<int> sd_pattern[4];  // snare drum pattern
int note_value[9] = {0,36,45,38,47,50,50,50,57};  //0 means no sound , [6]=>High Mid Tom , [7]=>High Low Tom
/*"No sound","Bass drum","Low tom","Snare drum","Mid tom","High tom","High Mid Tom","High Low Tom","Crash cymbal*/

class AIM // AI Music
{
public:
    AIM(string fname,int pnum,int gnum);
    int* GA_PROGRESS();
    int getFillInSize();
    void write_file(int *result);
private:
    void initailize();
    void selection_and_crossover();   //parent selection
    void corssover();
    void mutation();
    void survivor();    //survivor selection
    int fitness(int *ary);
    void termination();
    void read_file();

    vector<int> drum_pattern;
    string midiFileName;
    int pitch_num;
    int **individual;
    int **offspring_pool;
    int **individual_temp;
    int *fitness_array;
    int generation;
    int population_size;
    int fill_in_size; //usually 4 ,8 ,16 ,32

};

class ID_FITNESS_SET
{
public:
    int d_id;   //individual id
    int d_fit;  //individual fitness
    bool is_parent; //true => from individual , false => from offspring_pool
};

bool wayToSort(ID_FITNESS_SET f1, ID_FITNESS_SET f2)
{
    return f1.d_fit > f2.d_fit;
}

string itos(int num) //convert int to string
{
    stringstream ss;
    ss << num;
    return ss.str();
}


int main()
{
    cout<<"The program will generate a 16th-notes drum¡¦s fill-in pattern with GA,"<<endl;
    cout<<"Popular size is 80, program terminate at 2000 generation in default,"<<endl;
    cout<<"The fill-in pattern will be output to \"Fill-in.txt\ in MIDI ASCII format."<<endl<<endl;
    string input_file_name;
    ifstream fp;
    int i,*best;

    cout<<"Enter the input file name: ";
    while(cin>>input_file_name)
    {
        fp.open(input_file_name.c_str()); //open file
        if(!fp)
        {
            cout<<"Fail to open file: "<<input_file_name<<", please enter again: ";
            continue;
        }
        AIM g1(input_file_name,2,1);
        best = g1.GA_PROGRESS();
        g1.write_file(best);
        fp.close();
        file_content.clear();
        for(i=0;i<4;i++)
        {
            bd_pattern[i].clear();
            sd_pattern[i].clear();
        }
        cout<<"Enter the input file name: ";
    }

    return 0;
}

AIM::AIM(string fname,int pnum,int gnum)
{
    population_size = pnum;
    generation = gnum;
    midiFileName = fname;
    read_file();
    fill_in_size = 16;  //use sixteen sixteenth note
}

void AIM::read_file()
{
    ifstream fp;
    int i,j,note,i_temp;
    string s_temp,s_temp2;
    vector<string> temp_line;
    char line[500];
    fp.open(midiFileName.c_str()); //open file
    if(!fp)
    {
        cout<<"Fail to open file: "<<midiFileName<<endl;
        exit(1);
    }

    while(fp.getline(line,sizeof(line),'\n'))
    {
        istringstream stream(line);
        /*Track number +1 for adding fill in pattern*/
        if(strstr(line,"MFile"))
        {
            stream>>s_temp;
            s_temp2 = s_temp + " ";
            stream>>i_temp;
            s_temp2 = s_temp2 + itos(i_temp) + " ";
            stream>>i_temp;
            s_temp2 = s_temp2 + itos(i_temp+2) + " ";
            stream>>i_temp;
            s_temp2 = s_temp2 + itos(i_temp);
            stream.str(s_temp2);
        }
        /*1st bar content X 3 */
        if(strstr(line,"On")||strstr(line,"Off"))
            temp_line.push_back(line);
        else if(strstr(line,"TrkEnd"))
        {
            for(i=2;i<=2;i=i++)
            for(j=0;j<temp_line.size();j++)
            {
                temp_line[j][0] = i+'0';
                file_content.push_back(temp_line[j]);
            }
            temp_line.clear();
        }

        file_content.push_back(stream.str());
        if(strstr(line,"n=36")&&strstr(line,"On")) // get the bass drum click number
        {
            //cout<<line<<endl;
            stream>>s_temp;
            for(int i=0;i<s_temp.length();i++)
                if(s_temp[i]==':')
                    s_temp[i] = ' ';
            stream.str(s_temp);
            stream>>i_temp;
            stream>>i_temp; // note
            note = i_temp;
            stream>>i_temp; // click number
            bd_pattern[note].push_back(i_temp);
        }
        else if(strstr(line,"n=38")&&strstr(line,"On")) // get the snare drum click number
        {
            //cout<<line<<endl;
            stream>>s_temp;
            for(int i=0;i<s_temp.length();i++)
                if(s_temp[i]==':')
                    s_temp[i] = ' ';
            stream.str(s_temp);
            stream>>i_temp;
            stream>>i_temp; // note
            note = i_temp;
            stream>>i_temp; // click number
            sd_pattern[note].push_back(i_temp);
        }
    }
    /*for(i=0;i<file_content.size();i++)
        cout<<file_content[i]<<endl;*/
    fp.close();
}

void AIM::write_file(int *result)
{
    int bar=1,note=0,click_num=0,n,v=64;
    string str;
    vector<char> line;
    file_content.push_back("Mtrk");
    file_content.push_back("0:0:0 Meta Text \"Fill in\"");
    for(int i=0;i<fill_in_size;i++)
    {
        note = i/4;  //4 note in 1 beat , totally 4 beats
        n = note_value[result[i]];
        str = itos(bar)+":"+itos(note)+":"+itos(click_num)+" On ch=10 n="+itos(n)+" v="+itos(v);
        file_content.push_back(str);
        str = itos(bar)+":"+itos(note)+":"+itos(click_num+30)+" Off ch=10 n="+itos(n)+" v="+itos(v);
        file_content.push_back(str);
        click_num = (click_num+60)%240;
    }
    file_content.push_back("TrkEnd");
    /*Beat chord , High + Mid/Low Tom combination*/
    file_content.push_back("Mtrk");
    file_content.push_back("0:0:0 Meta Text \"Fill in Chord\"");
    click_num = 0;
    for(int i=0;i<fill_in_size;i++)
    {
        note = i/4;  //4 note in 1 beat , totally 4 beats
        if(result[i]==6||result[i]==7)
        {
            n = (result[i]==6)?45:47;
            str = itos(bar)+":"+itos(note)+":"+itos(click_num)+" On ch=10 n="+itos(n)+" v="+itos(v);
            file_content.push_back(str);
            str = itos(bar)+":"+itos(note)+":"+itos(click_num+30)+" Off ch=10 n="+itos(n)+" v="+itos(v);
            file_content.push_back(str);
        }
        click_num = (click_num+60)%240;
    }
    file_content.push_back("TrkEnd");

    /*for(int i=0;i<file_content.size();i++)
        cout<<file_content[i]<<endl;*/

    ofstream myfile("Fill-in.txt");
    if(!myfile.is_open())
    {
        cout<<"Fail to open example.txt: "<<endl;
        exit(1);
    }
    for(int i=0;i<file_content.size();i++)
        myfile<<file_content[i]<<endl;
    myfile.close();
}

int* AIM::GA_PROGRESS()
{
    int i,j,fit,max_fitness=INT_MIN;
    int *best_idv = new int[fill_in_size];

    initailize();
    while(generation--)
    {
        selection_and_crossover();
        mutation();
        survivor();
        fit = fitness(individual[0]);
        if(fit > max_fitness)
        {
            cout<<"The highest fitness point changed from"<<max_fitness<<" to "<<fit<<" at "<<2000-generation<<" generation."<<endl;
            max_fitness = fit;
            for(j=0;j<fill_in_size;j++)  //memcpy(&best_idv,&individual[0],sizeof(int)) wrong!
                best_idv[j] = individual[0][j];
        }
    }

    /*for(i=0;i<population_size;i++)
    {
        cout<<"individual"<<i<<" : ";
        for(j=0;j<fill_in_size;j++)
            cout<<individual[i][j]<<" ";
        cout<<"fitness is "<<fitness(individual[i])<<endl;
    }*/

    cout <<"The best individual over 2000 generations is :"<<endl;
    for(j=0;j<fill_in_size;j++)
            cout<<best_idv[j]<<" ";
        cout<<"fitness is "<<fitness(best_idv)<<endl<<endl;
    cout <<"Fill-in pattern is wrote into Fill-in.txt"<<endl;

    return best_idv;
}

void AIM::initailize()
{
     fitness_array = new int[population_size]; // initailize to zero
     for(int i=0;i<population_size;i++)
        fitness_array[i] = i;
     individual = new int*[population_size];
     offspring_pool = new int*[population_size];
     individual_temp = new int*[population_size];
     for(int i = 0; i < population_size; ++i)
     {
         individual[i] = new int[fill_in_size];
         offspring_pool[i] = new int[fill_in_size];
         individual_temp[i] = new int[fill_in_size];
     }

     /*Produce population_size x fill_in_size array of individual*/
     srand((unsigned)time(0));
     for(int i =0;i<population_size;i++)
        for(int j=0;j<fill_in_size;j++)
            individual[i][j] = rand()%INSTRUMENT_NUM;
}

void AIM::selection_and_crossover()
{
    int i,j,dad_index,mom_index,slice_head;
    vector<ID_FITNESS_SET> order_list;
    ID_FITNESS_SET ftemp;

    for(i=0;i<population_size/2;i++)
    {
        /*parent selection , random to be the parent from the random set*/
        dad_index = rand()%population_size;
        do{
            mom_index = rand()%population_size;
        }while(dad_index==mom_index);

        /*parent selection , best 2 of random 5*/
        /*order_list.clear();
        while(order_list.size()<5)
        {
            dad_index = rand()%population_size;
            ftemp.d_id = dad_index;
            ftemp.d_fit = fitness(individual[dad_index]);
            ftemp.is_parent = false;
            order_list.push_back(ftemp);
        }
        sort(order_list.begin(),order_list.end(),wayToSort);
        dad_index = order_list[0].d_id;
        mom_index = order_list[1].d_id;
        for(j=1;j<5&&order_list[j-1].d_id==order_list[j].d_id;j++)
            mom_index = order_list[j].d_id;*/

        /*1 point crossover , put offspring in offspring_pool*/
        slice_head = rand()%fill_in_size;
        for(j=0;j<slice_head;j++)
        {
            offspring_pool[2*i][j] = individual[dad_index][j];
            offspring_pool[2*i+1][j] = individual[mom_index][j];
        }
        for(;j<fill_in_size;j++)
        {
            offspring_pool[2*i][j] = individual[mom_index][j];
            offspring_pool[2*i+1][j] = individual[dad_index][j];
        }
    }
}


void AIM::mutation()
{
    /*Swap mutation with 1/L probability*/
    int i,j,mutation_sign,swap_index,temp;

    for(i=0;i<population_size;i++)
        for(j=0;j<fill_in_size;j++)
        {
            mutation_sign = rand()%fill_in_size;
            if(mutation_sign == j)
            {
                do{
                    swap_index = rand()%fill_in_size;
                }while(swap_index != j);
                temp = offspring_pool[i][j];
                offspring_pool[i][j] = offspring_pool[i][swap_index];
                offspring_pool[i][swap_index] = temp;
                //offspring_pool[i][j] = rand()%INSTRUMENT_NUM;
            }
        }
}

void AIM::survivor()
{
    /*Select the top half of the multiset of parents and offspring */
    vector<ID_FITNESS_SET> order_list;
    ID_FITNESS_SET temp;
    int i,j,new_id;
    for(i=0;i<population_size;i++)
    {
        temp.d_id = i;
        temp.d_fit = fitness(individual[i]);
        temp.is_parent = true;
        order_list.push_back(temp);
        temp.d_id = i;
        temp.d_fit = fitness(offspring_pool[i]);
        temp.is_parent = false;
        order_list.push_back(temp);
    }
    sort(order_list.begin(),order_list.end(),wayToSort);
    /*cout<<"after sort:"<<endl;
    for(i=0;i<order_list.size();i++)
        cout<<order_list[i].d_id<<" "<<order_list[i].d_fit<<" "<<order_list[i].is_parent<<endl;*/

    /*Put order individual into temp array*/
    for(i=0;i<population_size;i++)
    {
        new_id = order_list[i].d_id;
        if(order_list[i].is_parent)
        for(j=0;j<fill_in_size;j++)
            individual_temp[i][j] = individual[new_id][j];  //memcpy(&individual_temp[i],&individual[new_id],sizeof(int)) => got wrong!
        else
        for(j=0;j<fill_in_size;j++)
            individual_temp[i][j] = offspring_pool[new_id][j];  //memcpy(&individual_temp[i],&offspring_pool[new_id],sizeof(int)) wrong!
    }

    /*Survivor replace*/
    for(i=0;i<population_size;i++)
        for(j=0;j<fill_in_size;j++)
            individual[i][j] = individual_temp[i][j];   //memcpy(&individual[i],&individual_temp[i],sizeof(int)) wrong!
}

int AIM::fitness(int *ary)
{
    int i,j,fit=0,note,click_num = 0;
    int null_note[4] ={0};
    /*Rule 1*/
    for(i=fill_in_size-1;i>0;i--)
        for(j=i-1;j>=8;j--)
            if(ary[i]>ary[j]&&ary[i]!=8&&ary[j]!=0)
                fit -=100;
    for(i=0;i<fill_in_size;i++)
    {
        /*Rule 2*/
        if(ary[i]==8&&i==fill_in_size-1)
            fit += 10;
        else if(ary[i]==8&&(i+1)%4!=0)
            fit -= 10;
        /*if(ary[i]==8&&ary[i+1]!=0)
            fit -=10;*/

        /*Rule 3*/
        note = i/4;
        if(ary[i]==1)   //bass drum
            for(j=0;j<bd_pattern[note].size();j++)
                if(click_num == bd_pattern[note][j])
                    fit += 20;
                else
                    fit -= 10;
        /*Rule 4*/
        if(ary[i]==2)   //snare drum
            for(j=0;j<sd_pattern[note].size();j++)
                if(click_num != sd_pattern[note][j])
                    fit += 10;

        click_num = (click_num+60)%240;

        /*Rule 5*/
        if(ary[i]==0)
            null_note[note]++;
    }
    /*Rule 5*/
    /*for(i=0;i<4;i++)
        for(j=3;j>i;j--)
            if(null_note[i]==null_note[j]&&null_note[i]!=0)
                fit += 10;*/
    return fit;
}

int AIM::getFillInSize()
{
    return fill_in_size;
}
