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

    void simple_schedule(DayRequestData day_request_data, ServersData server_data, unordered_map <string, VMData> vms)
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
                // 查询A/B节点可用的CPU和内存
                int available_cpu_A = get_server_available_cpu_a(iter_server_id);         // 查询A节点可用的CPU
                int available_cpu_B = get_server_available_cpu_b(iter_server_id);         // 查询B节点可用的CPU
                int available_memory_A = get_server_available_memory_a(iter_server_id);   // 查询A节点可用的内存
                int available_memory_B = get_server_available_memory_b(iter_server_id);   // 查询B节点可用的内存

                // 查询虚拟机需要的CPU和内存
                int request_cpu = vms[day_request_data.add_req[j].vm_type].cpu;           // 查询虚拟机需要的CPU
                int request_memory = vms[day_request_data.add_req[j].vm_type].memory;     // 查询虚拟机需要的内存

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
                    if (((request_cpu / 2) <= available_cpu_A) &&  ((request_cpu / 2) <= available_cpu_B) &&
                        ((request_memory / 2) <= available_memory_A) && ((request_memory / 2) <= available_memory_B)) 
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
                else{
                    // 部署到A节点上
                    if ((request_cpu <= available_cpu_A) && (request_memory <= available_memory_A))
                    {
                        deploy_vm(vms[day_request_data.add_req[j].vm_type], iter_server_id, day_request_data.add_req[j].id, 'A');
                        deploy_output.append("(" + to_string(iter_server_id) + ", A)\n");
                        // cout << j << " after: A----" << to_string(get_server_available_cpu_a(iter_server_id)) << ", B----"
                        //                              << to_string(get_server_available_cpu_b(iter_server_id)) << ")." << endl;
                        break;
                    }
                    // 部署到B节点上
                    else if ((request_cpu <= available_cpu_B) && (request_memory <= available_memory_B))
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
            cout << "(purchase, " << 0 << ")" << endl;
        }
        else{
            cout << "(purchase, " << purchase_type <<  ")" << endl;
            cout << "(" << server_data.server_type << ", " << new_purchase << ")" << endl;
        }
        cout << "(migration, " << migration_times << ")" << endl;
        cout << deploy_output;
    }


  private:
    bool debug;
    DeployedVMData deployed_vm_data;
    int all_servers_num = 0;    // 服务器编号ID
    // DeployedVMData migrate_vm_data;
    
    // 得到对应id的服务器A节点可用CPU核数
    int get_server_available_cpu_a(int server_id)
    {
        return purchased_servers[server_id].available_cpu['A'];
    }

    // 得到对应id的服务器B节点可用CPU核数
    int get_server_available_cpu_b(int server_id)
    {
        return purchased_servers[server_id].available_cpu['B'];
    }

    // 得到对应id的服务器A节点可用内存
    int get_server_available_memory_a(int server_id)
    {
        return purchased_servers[server_id].available_memory['A'];
    }

    // 得到对应id的服务器B节点可用内存
    int get_server_available_memory_b(int server_id)
    {
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
        all_servers_num += 1;                                               // 只能购买，每买一台，总数加一
    }
    
    // 在服务器上部署虚拟机，指定vm_data----虚拟机，server_id----服务器id， request_id----请求任务的id，deploy_type----部署类型
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

    // 在服务器上删除虚拟机，指定server_id----服务器id， request_id----请求任务的id
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

    // 在服务器之间迁移虚拟机，指定old_server_id----被迁移虚拟机的服务器id， new_server_id----目标服务器id，request_id----要迁移虚拟机对应的请求任务的id
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