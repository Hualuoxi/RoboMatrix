#ifndef SIMPLE_SCHEDULE_HPP_
#define SIMPLE_SCHEDULE_HPP_

#include "data_handling.hpp"
#include <map>
#include "stdlib.h"
#include <string>
#include <vector>
using namespace std;


// 已部署的虚拟机数据类型
struct DeployedVMData
{
    int request_id;            // 用户请求的id
    char deploy_type;        // 部署类型：'D'----双；'A'----单A；'B'----单B
    VMData vm_data;
};

// 已经购买的服务器数据类型
struct PurchasedServersData
{
    /* data */
    ServersData server_data;   // 服务器参数
    map <int, DeployedVMData> deployed_vms;  // 部署在服务器上的虚拟机 
    map <char, int> available_cpu;         // 可用CPU大小, A/B节点
    map <char, int> available_memory;      // 可用内存大小, A/B节点
};

class Schedule
{
  public:
    map <int, PurchasedServersData> purchased_servers;    // 所有购买的服务器
    long long costs=0;
    // long long hardware_costs=0;
    long long energy_day_costs=0;
    long long energy_costs = 0;
    

    Schedule(bool _debug=true)
    {
        debug = _debug;
    };

    // 累计每天的成本
    long long get_costs()
    {
        long long hardware_costs = 0;
        for (long unsigned int i=0; i<purchased_servers.size(); i++){
            hardware_costs += purchased_servers[i].server_data.hardware_cost;
            if (purchased_servers[i].deployed_vms.size() != 0){
                energy_day_costs += purchased_servers[i].server_data.energy_day;
            }
        }
        costs = hardware_costs + energy_day_costs;
        return costs;
    }

    string simple_schedule(DayRequestData day_request_data, ServersData server_data, unordered_map <string, VMData> vms)
    {
        string all_output = "";              // 一天的操作输出
        string deploy_output = "";           // 部署时的输出语句
        int purchase_type = 1;        // 购买型号
        int new_purchase = 0;         // 购买台数
        int migration_times = 0;      // 迁移次数
        
        if (purchased_servers.size() == 0){   // 开局先买一台服务器
            purchase_servers(server_data);
            new_purchase += 1;
        }
        // 处理一天Add的数据
        for (long unsigned int j=0; j<day_request_data.add_req.size(); j++) {
            for (long unsigned int iter_server_id = 0; iter_server_id < purchased_servers.size(); iter_server_id ++)
            {
                // if (iter_server_id == 6){
                //     cout << "The type is " << day_request_data.add_req[j].vm_type << '\n' <<
                //             "The server id is " << iter_server_id<< '\n' <<
                //             "The max server id is " << all_servers_num << '\n' <<
                //             "The purchased_servers.size() is " << purchased_servers.size() << endl;
                //     cout << j << " before: A-CPU(" << to_string(get_server_available_cpu_a(6)) << "), B-CPU("
                //                                << to_string(get_server_available_cpu_b(6)) << ")." << endl;
                //     cout << j << " before: A-Memory(" << to_string(get_server_available_memory_a(6)) << "), B-Memory("
                //                                     << to_string(get_server_available_memory_b(6)) << ")." << endl;
                // }
                // cout << "Request ID-" << j << " before server ID-" << iter_server_id << ": A-CPU(" << to_string(get_server_available_cpu_a(iter_server_id)) << "), B-CPU("
                //                                                          << to_string(get_server_available_cpu_b(iter_server_id)) << ")." << endl;
                // cout << "Request ID-" << j << " before server ID-" << iter_server_id << ": A-Memory(" << to_string(get_server_available_memory_a(iter_server_id)) << "), B-Memory("
                //                                                             << to_string(get_server_available_memory_b(iter_server_id)) << ")." << endl;
                // cout << "The type is " << day_request_data.add_req[j].vm_type << '\n' <<
                //             "The server id is " << iter_server_id<< '\n' <<
                //             "The max server id is " << all_servers_num << '\n' <<
                //             "The purchased_servers.size() is " << purchased_servers.size() << endl;
                // cout << j << ": " << to_string(all_servers_num) + "\n";
                if (purchased_servers.size() > 750){
                    if (iter_server_id < 550) {
                        continue;
                    }
                }
                if (purchased_servers.size() > 1200){
                    if (iter_server_id < 900) {
                        continue;
                    }
                }
                if (purchased_servers.size() > 1500){
                    if (iter_server_id < 1200) {
                        continue;
                    }
                }
                if (purchased_servers.size() > 1800){
                    if (iter_server_id < 1500) {
                        continue;
                    }
                }
                // 双节点的添加
                if (vms[day_request_data.add_req[j].vm_type].node == 1) {
                    if (((vms[day_request_data.add_req[j].vm_type].cpu / 2) <= (get_server_available_cpu_a(iter_server_id))) && 
                        ((vms[day_request_data.add_req[j].vm_type].cpu / 2) <= (get_server_available_cpu_b(iter_server_id))) &&
                        ((vms[day_request_data.add_req[j].vm_type].memory / 2) <= (get_server_available_memory_a(iter_server_id))) &&
                        ((vms[day_request_data.add_req[j].vm_type].memory / 2) <= (get_server_available_memory_b(iter_server_id)))) 
                    {
                        deploy_vm(vms[day_request_data.add_req[j].vm_type], iter_server_id, day_request_data.add_req[j].id, 'D');
                        deploy_output.append("(" + to_string(iter_server_id) + ")\n");
                        // cout << j << " after: D----" << to_string(get_server_available_cpu_a(iter_server_id)) << endl;
                        break;
                    }
                    else{
                        if (iter_server_id != (purchased_servers.size() - 1)){
                            continue;
                        }
                        else{
                            purchase_servers(server_data);
                            new_purchase += 1;
                        }
                    }
                }
                // 单节点的添加：优先部署到A节点，不满足再部署B节点
                // else if (vms[day_request_data.add_req[j].vm_type].node == 0) {
                else{
                    // 部署到A节点上
                    if (((vms[day_request_data.add_req[j].vm_type].cpu) <= (get_server_available_cpu_a(iter_server_id))) &&
                        ((vms[day_request_data.add_req[j].vm_type].memory) <= (get_server_available_memory_a(iter_server_id))))
                    {
                        deploy_vm(vms[day_request_data.add_req[j].vm_type], iter_server_id, day_request_data.add_req[j].id, 'A');
                        deploy_output.append("(" + to_string(iter_server_id) + ", A)\n");
                        // cout << j << " after: A----" << to_string(get_server_available_cpu_a(iter_server_id)) << ", B----"
                        //                              << to_string(get_server_available_cpu_b(iter_server_id)) << ")." << endl;
                        break;
                    }
                    // 部署到B节点上
                    else if (((vms[day_request_data.add_req[j].vm_type].cpu) <= (get_server_available_cpu_b(iter_server_id))) &&
                             ((vms[day_request_data.add_req[j].vm_type].memory) <= (get_server_available_memory_b(iter_server_id))))
                    {
                        deploy_vm(vms[day_request_data.add_req[j].vm_type], iter_server_id, day_request_data.add_req[j].id, 'B');
                        deploy_output.append("(" + to_string(iter_server_id) + ", B)\n");
                        // cout << j << " after: A----" << to_string(get_server_available_cpu_a(iter_server_id)) << ", B----"
                        //                              << to_string(get_server_available_cpu_b(iter_server_id)) << "." << endl;
                        break;
                    }
                    else{
                        if (iter_server_id != (purchased_servers.size() - 1)){
                            continue;
                        }
                        else{
                            purchase_servers(server_data);
                            new_purchase += 1;
                        }
                    }
                }
                // cout << "Request ID-" << j << " after server ID-" << iter_server_id << ": A-CPU(" << to_string(get_server_available_cpu_a(iter_server_id)) << "), B-CPU("
                //                                                          << to_string(get_server_available_cpu_b(iter_server_id)) << ")." << endl;
                // cout << "Request ID-" << j << " after server ID-" << iter_server_id << ": A-Memory(" << to_string(get_server_available_memory_a(iter_server_id)) << "), B-Memory("
                //                                                             << to_string(get_server_available_memory_b(iter_server_id)) << ").\n" << endl;
                
                // continue;
            }
        }
        
        // 处理一天Del的数据
        for (int j=0; j<day_request_data.del_req.size(); j++) {
            for (long unsigned int iter_server_id=0; iter_server_id<purchased_servers.size(); iter_server_id++)
            {
                if (purchased_servers[iter_server_id].deployed_vms.count(day_request_data.del_req[j].id)) {
                    del_vm(iter_server_id, day_request_data.del_req[j].id);
                }
            }
        }


        
        if (new_purchase == 0) {
            all_output.append("(purchase, " + to_string(0) + ")\n");   // 输出购买型号
        }
        else{
            all_output.append("(purchase, " + to_string(purchase_type) + ")\n");   // 输出购买型号
            all_output.append("(" + server_data.server_type + ", " + to_string(new_purchase) + ")\n"); // 购买服务器数量
        }
        all_output.append("(migration, " + to_string(migration_times) + ")\n");
        all_output.append(deploy_output);

        return all_output;
    }


  private:
    bool debug;
    DeployedVMData deployed_vm_data;
    int all_servers_num = 0;    // 服务器编号ID
    // DeployedVMData migrate_vm_data;
    
    // 以下得到各节点可用CPU核数和内存，可根据实际情况改写
    int get_server_available_cpu_a(int server_id)
    {
        // int available_cpu = purchased_servers[server_id].server_data.cpu/2;
        // // 遍历部署的虚拟机
        // map<int, DeployedVMData>::iterator iter_cpu_a;
        // for (iter_cpu_a=purchased_servers[server_id].deployed_vms.begin(); iter_cpu_a!=purchased_servers[server_id].deployed_vms.end(); iter_cpu_a++)
        // {
        //     //iter_cpu_a->first----key值，iter_cpu_a->second----value值
        //     if (iter_cpu_a->second.deploy_type == 'D'){
        //         available_cpu -= iter_cpu_a->second.vm_data.cpu / 2;
        //     }
        //     else if (iter_cpu_a->second.deploy_type == 'A') {
        //         available_cpu -= iter_cpu_a->second.vm_data.cpu;
        //     }
        //     else{
        //         continue;
        //     }
        //     // cout << available_cpu << endl;
        // }
        // return available_cpu;
        return purchased_servers[server_id].available_cpu['A'];
    }
    int get_server_available_cpu_b(int server_id)
    {
        // int available_cpu = purchased_servers[server_id].server_data.cpu/2;
        // // cout << "\n" << available_cpu << endl;
        // // 遍历部署的虚拟机
        // map<int, DeployedVMData>::iterator iter_cpu_b;
        // for (iter_cpu_b=purchased_servers[server_id].deployed_vms.begin(); iter_cpu_b!=purchased_servers[server_id].deployed_vms.end(); iter_cpu_b++)
        // {
        //     // cout <<  "\t" << "Server-ID-" << server_id << "'s vms type are: " << iter_cpu_b->second.deploy_type << endl;
        //     // cout << iter_cpu_b->second.deploy_type << endl;
        //     // iter_cpu_b->first----key值，iter_cpu_b->second----value值
        //     if (iter_cpu_b->second.deploy_type == 'D'){
        //         available_cpu -= iter_cpu_b->second.vm_data.cpu / 2;
        //     }
        //     else if (iter_cpu_b->second.deploy_type == 'B') {
        //         available_cpu -= iter_cpu_b->second.vm_data.cpu;
        //     }
        //     else{
        //         continue;
        //     }
        //     // cout << available_cpu << endl;
        // }
        // // cout << available_cpu << endl;
        // return available_cpu;
        return purchased_servers[server_id].available_cpu['B'];
    }
    int get_server_available_memory_a(int server_id)
    {
        // int available_memory = purchased_servers[server_id].server_data.memory/2;
        // // 遍历部署的虚拟机
        // map<int, DeployedVMData>::iterator iter_memory_a;
        // for (iter_memory_a=purchased_servers[server_id].deployed_vms.begin(); iter_memory_a!=purchased_servers[server_id].deployed_vms.end(); iter_memory_a++)
        // {
        //     //iter_memory_a->first----key值，iter_memory_a->second----value值
        //     if (iter_memory_a->second.deploy_type == 'D'){
        //         available_memory -= iter_memory_a->second.vm_data.memory / 2;
        //     }
        //     else if (iter_memory_a->second.deploy_type == 'A') {
        //         available_memory -= iter_memory_a->second.vm_data.memory;
        //     }
        //     else{
        //         continue;
        //     }
        // }
        // return available_memory;
        return purchased_servers[server_id].available_memory['A'];
    }
    int get_server_available_memory_b(int server_id)
    {
        // int available_memory = purchased_servers[server_id].server_data.memory/2;
        // // 遍历部署的虚拟机
        // map<int, DeployedVMData>::iterator iter_memory_b;
        // for (iter_memory_b=purchased_servers[server_id].deployed_vms.begin(); iter_memory_b!=purchased_servers[server_id].deployed_vms.end(); iter_memory_b++)
        // {
        //     //iter_memory_b->first----key值，iter_memory_b->second----value值
        //     if (iter_memory_b->second.deploy_type == 'D'){
        //         available_memory -= iter_memory_b->second.vm_data.memory / 2;
        //     }
        //     else if (iter_memory_b->second.deploy_type == 'B') {
        //         available_memory -= iter_memory_b->second.vm_data.memory;
        //     }
        //     else{
        //         continue;
        //     }
        // }
        // return available_memory;
        return purchased_servers[server_id].available_memory['B'];
    }

    // 购买服务器
    void purchase_servers(ServersData server_data)
    {
        PurchasedServersData purchased_servers_data;
        purchased_servers_data.server_data = server_data;
        // A、B节点分别是总CPU核数和总内存的一半
        purchased_servers_data.available_cpu['A'] = server_data.cpu / 2;
        purchased_servers_data.available_memory['A'] = server_data.memory / 2;
        purchased_servers_data.available_cpu['B'] = server_data.cpu / 2;
        purchased_servers_data.available_memory['B'] = server_data.memory / 2;
        purchased_servers[all_servers_num] = purchased_servers_data;
        // cout << "\n增加服务器：id-" << all_servers_num << '\n' << 
        //             "The max server id is " << all_servers_num << '\n' <<
        //             "The CPU-A is " << purchased_servers_data.available_cpu['A'] << '\n' <<
        //             "The CPU-B is " << purchased_servers_data.available_cpu['B'] << '\n' <<
        //             "The Memory-A is " << purchased_servers_data.available_memory['A'] << '\n' <<
        //             "The Memory-B is " << purchased_servers_data.available_memory['B'] << '\n' << endl;
        all_servers_num += 1;                                               // 只能购买，每买一台，总数加一
    }

    void deploy_vm(VMData vm_data, int server_id, int request_id, char deploy_type)
    {
    // 指定服务器ID和虚拟器进行部署
    // 分为双节点、单节点部署，单节点又分为A、B节点部署
    // 可用cpu核数是现可用cpu核数减去部署的虚拟机的cpu核数
    // 可用内存是现可用内存减去部署的虚拟机的内存
        deployed_vm_data.request_id = request_id;
        deployed_vm_data.deploy_type = deploy_type;
        deployed_vm_data.vm_data = vm_data;
        purchased_servers[server_id].deployed_vms[request_id] = deployed_vm_data;
        // 双节点部署,当服务器是双节点时必须双节点部署
        if (deploy_type == 'D' || vm_data.node == 1){
            purchased_servers[server_id].available_cpu['A'] -= vm_data.cpu / 2;               
            purchased_servers[server_id].available_memory['A'] -= vm_data.memory / 2;         
            purchased_servers[server_id].available_cpu['B'] -= vm_data.cpu / 2;      
            purchased_servers[server_id].available_memory['B'] -= vm_data.memory / 2;
        }
        // 单A节点部署
        else if (deploy_type == 'A' || deploy_type == 'a'){                   
            purchased_servers[server_id].available_cpu['A'] -= vm_data.cpu;               
            purchased_servers[server_id].available_memory['A'] -= vm_data.memory;
        }
        // 单B节点部署
        else if (deploy_type == 'B' || deploy_type == 'b'){                   
            purchased_servers[server_id].available_cpu['B'] -= vm_data.cpu;            
            purchased_servers[server_id].available_memory['B'] -= vm_data.memory;
        }
    }

    void del_vm(int server_id, int request_id)
    {
    // 删掉服务器上的虚拟机，记得写空集错误
        // 双节点
        if (purchased_servers[server_id].deployed_vms[request_id].deploy_type == 'D'){
            purchased_servers[server_id].available_cpu['A'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.cpu / 2;               
            purchased_servers[server_id].available_memory['A'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.memory / 2;         
            purchased_servers[server_id].available_cpu['B'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.cpu / 2;      
            purchased_servers[server_id].available_memory['B'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.memory / 2;
        }
        // 单A节点
        else if (purchased_servers[server_id].deployed_vms[request_id].deploy_type == 'A'){                   
            purchased_servers[server_id].available_cpu['A'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.cpu;               
            purchased_servers[server_id].available_memory['A'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.memory;
        }
        // 单B节点
        else if (purchased_servers[server_id].deployed_vms[request_id].deploy_type == 'B'){                   
            purchased_servers[server_id].available_cpu['B'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.cpu;            
            purchased_servers[server_id].available_memory['B'] += purchased_servers[server_id].deployed_vms[request_id].vm_data.memory;
        }

        purchased_servers[server_id].deployed_vms.erase(request_id);

    }

    void migrate_vm(int old_server_id, int new_server_id, int request_id)
    {
        purchased_servers[new_server_id].deployed_vms[request_id] = purchased_servers[old_server_id].deployed_vms[request_id];
        purchased_servers[old_server_id].deployed_vms.erase(request_id);

        if (purchased_servers[new_server_id].deployed_vms[request_id].deploy_type == 'D'){
            purchased_servers[new_server_id].available_cpu['A'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu / 2;               
            purchased_servers[new_server_id].available_memory['A'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory / 2;         
            purchased_servers[new_server_id].available_cpu['B'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu / 2;      
            purchased_servers[new_server_id].available_memory['B'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory / 2;
            purchased_servers[old_server_id].available_cpu['A'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu / 2;               
            purchased_servers[old_server_id].available_memory['A'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory / 2;         
            purchased_servers[old_server_id].available_cpu['B'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu / 2;      
            purchased_servers[old_server_id].available_memory['B'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory / 2;
        }
        // 单A节点
        else if (purchased_servers[new_server_id].deployed_vms[request_id].deploy_type == 'A'){                   
            purchased_servers[new_server_id].available_cpu['A'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu;               
            purchased_servers[new_server_id].available_memory['A'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory;
            purchased_servers[old_server_id].available_cpu['A'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu;               
            purchased_servers[old_server_id].available_memory['A'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory;
        }
        // 单B节点
        else if (purchased_servers[new_server_id].deployed_vms[request_id].deploy_type == 'B'){                   
            purchased_servers[new_server_id].available_cpu['B'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu;            
            purchased_servers[new_server_id].available_memory['B'] += purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory;
            purchased_servers[old_server_id].available_cpu['B'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.cpu;            
            purchased_servers[old_server_id].available_memory['B'] -= purchased_servers[new_server_id].deployed_vms[request_id].vm_data.memory;
        }
    }
    
};



#endif