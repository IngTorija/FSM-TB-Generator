#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <sstream>      // std::stringstream
#include <regex>
#include <string>
#include <vector> 

using namespace std;
string pat_name = "(module) (.+)";
string pat_inouts = "(input|(output)) (((\\[\\d\\:\\d\\].(\\w+,.+?(?=,)|\\w+,.+?(?=;)|\\w+?(?=;)|(\\w+?(?=,)))))|((\\w+,.+?(?=;))|(\\w+,.+?(?=\\n))|(\\w.+?(?=\\,))|(\\w?(?=\\n))|(\\w.?(?=;))|(\\w.+?(?=;))|(\\w.+?(?=\\r))))";
regex re_module_name(pat_name);
regex re_inouts(pat_inouts);

class TB
{
  public:
    /* File to write: */
    ofstream tb_write;
    /* Module names */
    string fileName;
    string formatName;
    string tb_module_name;
    string top_module_name;
    string ClockName;
    string ResetName;

    /*Data to find Closed loops*/
    vector<string> PS;
    vector<string> NS;
    vector<string> Stimulus;
    vector<vector<string>> closed_loops;
    vector<vector<string>> jump_conditions;
    vector<string> final_states;
    string reset_state;

    /* Input and output lists */
    vector <string> inputs;
    vector <string> outputs;
    vector <string> inputs_wo_bus;
    vector <string> outputs_wo_bus;
    vector <string> sizes_w_bus;
    vector <string> single_inputs;
    vector <string> bus_inputs;
    vector <string> total_sizes;
    

    TB(/* args */);
    //Read File
    void read_file();
    void read_format();

    void get_name(string fileName);
    void search_inouts(string fileName);
    void remove_bus(string fileName);
    void separate_bus(string fileName);
    
    //Write file methods:
    void write_file();
    void write_module_name();
    void write_inputs();
    void write_instantiation();
    void write_initial();
    void write_stimulus();
    void write_end();
    void write_fsm_process();
    void write_clock();
    void write_reset();
    

    //Find bits combinations
    vector<string> combinations;
    void findBitCombinations(int);

    //Find closed loops
    void find_closed_loops();
};

TB::TB(/* args */){}

void TB::read_file(){
  //get module name
  get_name(fileName);
  //Get inputs and outputs
  search_inouts(fileName);

  remove_bus(fileName);
  separate_bus(fileName);
  find_closed_loops();
}

void TB::get_name(string fileName){
    string line_temp;
    ifstream f;
    
    f.open (fileName);
        if ( f.good() ) {
            while (!f.eof()){
                getline(f,line_temp);
                smatch matches;
                if(regex_search(line_temp,matches,regex ("module (.+?(?=\\())"))){
                    
                    // Extract top module name
                    top_module_name = matches[1];
                    // Create TB module name
                    tb_module_name = top_module_name + "_TB";
                    break;
                }else cout << "No match" << endl;
            }
        }
}


void TB::search_inouts(string fileName){
    string pattern = "(input|(output reg)) (((\\[\\d\\:\\d\\].(\\w+,.+|\\w+,.+|\\w+|(\\w+))))|((\\w+,.+)|(\\w+,.+)|(\\w.+)|(\\w)|(\\w.)|(\\w.+)|(\\w.+)))";//"(input|(output)) (((\\[\\d\\:\\d\\].(\\w+,.+?(?=,)|\\w+,.+?(?=;)|\\w+?(?=;)|(\\w+?(?=,)))))|((\\w+,.+?(?=;))|(\\w+,.+?(?=\\n))|(\\w.+?(?=\\,))|(\\w?(?=\\n))|(\\w.?(?=;))|(\\w.+?(?=;))|(\\w.+?(?=\\n))))";
    string line_temp;
    ifstream f;
    string string_temp;
    f.open (fileName);
        if ( f.good() ) {
            while (!f.eof()){
                getline(f,line_temp);
                smatch matches;
                if(regex_search(line_temp, matches, regex(pattern))){
                    if(matches[1]=="input"){
                      string_temp = regex_replace(matches[3].str(),regex(";")," ");
                      string_temp = regex_replace(matches[3].str(),regex(",")," ");
                      inputs.push_back(string_temp);
                    }
                    else if (matches[1]=="output reg"){
                      string_temp = regex_replace(matches[3].str(),regex(";")," ");
                      string_temp = regex_replace(matches[3].str(),regex(",")," ");
                      outputs.push_back(string_temp);
                    }   
                }
            }
        }
}

void TB::remove_bus(string fileName){
  smatch matches;
  int i = 0;
  for(i = 0; i<inputs.size();i++){
    if(regex_search(inputs[i],matches,regex("(^\\[.+\\])(.+$)"))){
      inputs_wo_bus.push_back(matches[2]);
    }
    else{
      inputs_wo_bus.push_back(inputs[i]);
    }

  }
 for(i = 0; i<outputs.size();i++){
    if(regex_search(outputs[i],matches,regex("(^\\[.+\\])(.+$)"))){
      outputs_wo_bus.push_back(matches[2]);
    }
    else{
      outputs_wo_bus.push_back(outputs[i]);
    }
  }
}

void TB::separate_bus(string fileName){
  smatch matches;
for(int i = 0; i<inputs.size();i++){
  if (regex_search(inputs[i],matches,regex ("(^\\[.+\\]) (.+$)"))){
        sizes_w_bus.push_back(matches[1]); // [1,1,[3:0],1]
        bus_inputs.push_back(matches[2]); // [D]
      }else{
        sizes_w_bus.push_back("1");
        single_inputs.push_back(inputs[i]);// [A,B,C]
      }
}
}

void TB::write_file(){
  tb_write.open(tb_module_name+".sv");
  write_module_name();
  write_inputs();
  write_instantiation();
  write_initial();
 // write_stimulus();
  write_fsm_process();
  write_end();
  write_clock();

}

void TB::write_module_name(){
  tb_write << "module " + tb_module_name + ";\n";
}

void TB::write_inputs(){

  for(int i=0; i<inputs.size();i++)
  {
    tb_write << ("  reg ");
    tb_write << inputs[i] + ";\n";

  }
  for(int i=0; i<outputs.size();i++)
  {
    tb_write << ("  wire ");
    tb_write << outputs[i] + ";\n";

  }

}

void TB::write_instantiation(){
  tb_write<< "\r\n  " << top_module_name + " UUT(";
  for(int i = 0; i<inputs_wo_bus.size();i++)
    tb_write << inputs_wo_bus[i] + ",";

  for(int i = 0; i<outputs_wo_bus.size();i++)
    tb_write << outputs_wo_bus[i] + ",";
tb_write << ");\r\n";
}

void TB::write_initial(){
  tb_write << "\r\n initial begin \r\n";
  tb_write << "   $dumpfile(\"" + top_module_name +".vcd\");\n" +
              "   $dumpvars(1," + tb_module_name + "); \n";
}

void TB::write_end(){
  tb_write << "\n   $finish; \n end\n"<<endl;
}

void TB:: write_clock(){
  tb_write << "\ninitial  "<<endl;
  tb_write << " begin"<<endl;
  tb_write << "   "+ ClockName <<"=1'b0;"<<endl;
  tb_write << "   forever"<<endl;
  tb_write << "   #1 "+ ClockName +"= ~"+ ClockName +";"<<endl;
  tb_write << " end"<<endl;
  tb_write << "endmodule"<<endl;  
}

void TB::write_reset(){
  tb_write << "   "+ResetName<<"=1'b0;"<<endl;
  tb_write << "   #1"<<endl;
  tb_write << "   "+ResetName<<"=1'b1;"<<endl;
  tb_write << "   #1"<<endl;
  tb_write << "   "+ResetName<<"=1'b0;"<<endl;

}

void TB::write_fsm_process(){
  write_reset();
  for (int i=0; i<jump_conditions.size();i++)
  {
    tb_write << "//Path "<< to_string(i)<<endl;
    for(int j=0; j<jump_conditions[i].size();j++)
      if(jump_conditions[i][j] == "-")
        tb_write << "    #1"<<endl;
      else
        tb_write <<"    #1\n    "<<jump_conditions[i][j]<<";"<<endl;
  write_reset();
  }
}



void TB::find_closed_loops(){
    int i = 0;
    vector <string> loop_temp;
    vector <string> stimulus_temp;
    string final_state_temp = reset_state;
    bool ns_end_loop;
    int final_states_pos = 0;

    while(final_states.size()>1)
    //for (int x = 0;x<22;x++)
    {

      cout << "----------------------------------------------------------"<<endl;
        if(NS[i] == reset_state)
        {
          //loop_temp.push_back(PS[i]);
          loop_temp.push_back(NS[i]);
          stimulus_temp.push_back(Stimulus[i]);
          closed_loops.push_back(loop_temp);
          jump_conditions.push_back(stimulus_temp);
          loop_temp.clear();
          stimulus_temp.clear();
          final_state_temp = PS[i];
          PS.erase(PS.begin()+i);
          NS.erase(NS.begin()+i);
          Stimulus.erase(Stimulus.begin()+i); 
          i=0;          
        }
        else
        {
          for(int j = 0; j<final_states.size();j++)
          {
            if(NS[i] == final_states[j]){
              ns_end_loop = true;
              final_states_pos = j;
             break; 
            }
            else
            {
              ns_end_loop = false; 
            }
          }
          cout << to_string(ns_end_loop) <<endl;
          if(ns_end_loop)
          {
            if(PS[i]==NS[i])
            {
              //loop_temp.push_back(PS[i]);
              loop_temp.push_back(NS[i]);
              stimulus_temp.push_back(Stimulus[i]);
              closed_loops.push_back(loop_temp);
              jump_conditions.push_back(stimulus_temp);
              loop_temp.clear();
              stimulus_temp.clear();
              final_states.erase(final_states.begin()+final_states_pos);
              PS.erase(PS.begin()+i);
              NS.erase(NS.begin()+i);
              Stimulus.erase(Stimulus.begin()+i); 
              i=0;


            }
            else
            {
              //loop_temp.push_back(PS[i]);
              loop_temp.push_back(NS[i]);
              stimulus_temp.push_back(Stimulus[i]);
              for(int k = 0;k<PS.size();k++)
              {
                if(PS[k]==NS[i])
                {
                  i=k;
                  break;
                }
              }

            }
          }
          else
          {
            if(NS[i]==final_state_temp)
            {
              loop_temp.clear();
              stimulus_temp.clear();
              final_state_temp = PS[i];
              PS.erase(PS.begin()+i);
              NS.erase(NS.begin()+i);
              Stimulus.erase(Stimulus.begin()+i);
              i=0;
            }
            else
            {
              //loop_temp.push_back(PS[i]);
              loop_temp.push_back(NS[i]);
              stimulus_temp.push_back(Stimulus[i]);
              for(int k = 0;k<PS.size();k++)
              {
                if(PS[k]==NS[i])
                {
                  i=k;
                  break;
                }
              }

            }

          }
        }
    }

}

