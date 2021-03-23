#include "data_handling.h"

DataHandling::DataHandling(bool _debug = true )
{
    debug = _debug;
}
DataHandling::~DataHandling(void)
{

}

bool DataHandling::dealLineData(string tmp_line)
{
    if(debug) cout<< i <<"\n"; i=i+1;
    if(tmp_line.data()[0] != '(' ) //
    {
        if(num == 1)    
            servers_num = stoi(tmp_line);
        if(num == 2) 
            vms_num = stoi(tmp_line);
        if(num == 3) 
        {
            day_num = stoi(tmp_line);
            requests_all = new vector<DayRequestData>(day_num);
        }
        if(num > 3 ) 
        {
            everyday_num.push_back( stoi(tmp_line) );
            day_tmp++;
        }
        num+=1;
        if(debug) cout<<"num:  "<<stoi(tmp_line) <<"\n";
        return false;
    }
    else
    {
        p = tmp_line.begin();
        tmp_line.erase(p);  
        p = tmp_line.end(); p--;
        tmp_line.erase(p); 
        stringstream words(tmp_line);
        string tep_word;
        if(debug) cout <<"line:  " <<tmp_line <<endl;
        ser_tmp = 5; vm_tmp = 4;req_tmp=3;
        while(getline(words, tep_word, ','))//以,为分隔符，读取数据
        {
            if(num == 2) serversDeal(tep_word);
            if(num == 3) vmsDeal(tep_word);
            if(num>3)  requestsDeal(tep_word);
        }

        if(num ==2) 
        {
            servers[servers_data.server_type] = servers_data;
        }
        if(num == 3)
        {
            // pair<string,VMData>tmp (vms_data.vm_type,vms_data);
            // vms.insert(tmp);
            vms[vms_data.vm_type] = vms_data;
        }
        
        if (day_tmp > 0)
        {
            if (everyday_num.at(day_tmp - 1) != 0)  //当天请求数据不为0
            {
                requests_all->at(day_tmp - 1).day_request.push_back(req_data);
                if (req_data.req == "add")
                {
                    requests_all->at(day_tmp - 1).add_req.push_back(req_data);
                    requests_all->at(day_tmp - 1).DayNow_CPU += vms[req_data.vm_type].cpu;
					requests_all->at(day_tmp - 1).DayNow_Mem += vms[req_data.vm_type].memory;
					Now_CPU += vms[req_data.vm_type].cpu;
					Now_Mem += vms[req_data.vm_type].memory;
                }
                else
                {
                    requests_all->at(day_tmp - 1).del_req.push_back(req_data);
                    requests_all->at(day_tmp - 1).DayNow_CPU -= vms[req_data.vm_type].cpu;
					requests_all->at(day_tmp - 1).DayNow_Mem -= vms[req_data.vm_type].memory;
					Now_CPU -= vms[req_data.vm_type].cpu;
					Now_Mem -= vms[req_data.vm_type].memory;
                }
                requests_all->at(day_tmp - 1).DayPeak_CPU = ((requests_all->at(day_tmp - 1).DayPeak_CPU) > (requests_all->at(day_tmp - 1).DayNow_CPU)) ? (requests_all->at(day_tmp - 1).DayPeak_CPU) : (requests_all->at(day_tmp - 1).DayNow_CPU);
				requests_all->at(day_tmp - 1).DayPeak_Mem = ((requests_all->at(day_tmp - 1).DayPeak_Mem) > (requests_all->at(day_tmp - 1).DayNow_Mem)) ? (requests_all->at(day_tmp - 1).DayPeak_Mem) : (requests_all->at(day_tmp - 1).DayNow_Mem);
				Peak_CPU = (Peak_CPU > Now_CPU) ? Peak_CPU : Now_CPU;
				Peak_Mem = (Peak_Mem > Now_Mem) ? Peak_Mem : Now_Mem;
            }
            
            //当读到最后一天的最后一条数据  返回真
            if(day_tmp == day_num && requests_all->back().day_request.size() == everyday_num.back()) 
                return true;
        }

        if(debug) cout<<"\n";
        return false;
    }
}

bool DataHandling::openFile(const char *filePath)
{
    ifstream infile(filePath,std::ios::in);
    string tmp_line;
    
    cout <<"path: " <<filePath << "\n";
    
    if (!infile.fail())
    {          
        while(getline(infile, tmp_line ) && !infile.eof())  //逐行读取
        {
            dealLineData(tmp_line);
        }
        cout<<"read over\n";
    }
    else
    {
        return false;
    }
    
    infile.close();
    cout<<"close file\n";
    return true;
}

void DataHandling::serversDeal(string _word)
{
    if(ser_tmp == 5){
        servers_data.server_type = _word;
        ser_tmp--;
        if(debug) cout << servers_data.server_type<<"  ";
    }
    else if(ser_tmp == 4){
        servers_data.cpu = stoi(_word);
        ser_tmp--;
        if(debug) cout << servers_data.cpu<<"  ";
    }
    else if(ser_tmp == 3){
        servers_data.memory = stoi(_word);
        ser_tmp--;
        if(debug) cout << servers_data.memory<<"  ";
    }
    else if(ser_tmp == 2){
        servers_data.hardware_cost = stoi(_word);
        ser_tmp--;
        if(debug) cout << servers_data.hardware_cost<<"  ";
    }
    else if(ser_tmp == 1){
        servers_data.energy_day = stoi(_word);
        ser_tmp--;
        if(debug) cout << servers_data.energy_day<<"  ";
    }
}

void DataHandling::vmsDeal(string _word)
{
    if(vm_tmp == 4){
        vms_data.vm_type = _word;
        vm_tmp--;
        if(debug) cout << vms_data.vm_type<<"  ";
    }
    else if(vm_tmp == 3){
        vms_data.cpu = stoi(_word);
        vm_tmp--;
        if(debug) cout << vms_data.cpu<<"  ";
    }
    else if(vm_tmp == 2){
        vms_data.memory = stoi(_word);
        vm_tmp--;
        if(debug) cout << vms_data.memory<<"  ";
    }
        else if(vm_tmp == 1){
        vms_data.node = stoi(_word);
        vm_tmp--;
        if(debug) cout << vms_data.node<<"  ";
    }
}


void DataHandling::requestsDeal(string _word)
{
    if(req_tmp == 3){
        req_data.req = _word;
        req_tmp--;
        if(debug) cout<<"req: " << req_data.req <<"...";
    }
    else if(req_tmp == 2)
    {
        if(req_data.req == "add")
        {
            _word.erase(0,_word.find_first_not_of(" "));  //去年首部空格
            req_data.vm_type = _word;
            req_tmp--;
            if(debug)  cout<<"type: " << req_data.vm_type<<"...";
        }
        else
        {
            req_data.id = stoi(_word);
            if(debug)  cout<<"id: " << req_data.id <<"...";
        } 
    }
    else if(req_tmp == 1)
    {
        req_data.id = stoi(_word);
        if(debug) cout <<"id: "<< req_data.id <<"...";
    }
}