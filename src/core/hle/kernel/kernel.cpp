// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/kernel/client_port.h"
#include "core/hle/kernel/config_mem.h"
#include "core/hle/kernel/handle_table.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/memory.h"
#include "core/hle/kernel/process.h"
#include "core/hle/kernel/resource_limit.h"
#include "core/hle/kernel/shared_page.h"
#include "core/hle/kernel/thread.h"
#include "core/hle/kernel/timer.h"

namespace Kernel {

/// Initialize the kernel
KernelSystem::KernelSystem(Memory::MemorySystem& memory, Core::TimingManager& timing,
                           std::function<void()> prepare_reschedule_callback, u32 system_mode,
                           u32 num_cores)
    : memory(memory), timing(timing),
      prepare_reschedule_callback(std::move(prepare_reschedule_callback)) {
    MemoryInit(system_mode);

    resource_limits = std::make_unique<ResourceLimitList>(*this);
    for (u32 core_id = 0; core_id < num_cores; ++core_id) {
        thread_managers.push_back(std::make_unique<ThreadManager>(*this, core_id));
    }
    timer_manager = std::make_unique<TimerManager>(timing);
    stored_processes.assign(num_cores, nullptr);
}

/// Shutdown the kernel
KernelSystem::~KernelSystem() {
    ThreadManager::ResetThreadIDs();
};

ResourceLimitList& KernelSystem::ResourceLimit() {
    return *resource_limits;
}

const ResourceLimitList& KernelSystem::ResourceLimit() const {
    return *resource_limits;
}

u32 KernelSystem::GenerateObjectID() {
    return next_object_id++;
}

std::shared_ptr<Process> KernelSystem::GetCurrentProcess() const {
    return current_process;
}

void KernelSystem::SetCurrentProcess(std::shared_ptr<Process> process) {
    current_process = process;
    SetCurrentMemoryPageTable(&process->vm_manager.page_table);
}

void KernelSystem::SetCurrentProcessForCPU(std::shared_ptr<Process> process, u32 core_id) {
    if (current_cpu->id == core_id) {
        current_process = process;
        SetCurrentMemoryPageTable(&process->vm_manager.page_table);
    } else {
        stored_processes[core_id];
    }
}

void KernelSystem::SetCurrentMemoryPageTable(Memory::PageTable* page_table) {
    memory.SetCurrentPageTable(page_table);
    if (current_cpu != nullptr) {
        current_cpu->PageTableChanged(); // notify the CPU the page table in memory has changed
    }
}

void KernelSystem::SetCPUs(std::vector<std::shared_ptr<ARM_Interface>> cpus) {
    ASSERT(cpus.size() == thread_managers.size());
    u32 i = 0;
    for (auto cpu : cpus) {
        thread_managers[i++]->SetCPU(*cpu);
    }
}

void KernelSystem::SetRunningCPU(std::shared_ptr<ARM_Interface> cpu) {
    if (current_process) {
        stored_processes[current_cpu->id] = current_process;
    }
    current_cpu = cpu;
    timing.SetCurrentTimer(cpu->id);
    if (stored_processes[current_cpu->id]) {
        SetCurrentProcess(stored_processes[current_cpu->id]);
    }
}

ThreadManager& KernelSystem::GetThreadManager(u32 core_id) {
    return *thread_managers[core_id];
}

const ThreadManager& KernelSystem::GetThreadManager(u32 core_id) const {
    return *thread_managers[core_id];
}

ThreadManager& KernelSystem::GetCurrentThreadManager() {
    return *thread_managers[current_cpu->id];
}

const ThreadManager& KernelSystem::GetCurrentThreadManager() const {
    return *thread_managers[current_cpu->id];
}

TimerManager& KernelSystem::GetTimerManager() {
    return *timer_manager;
}

const TimerManager& KernelSystem::GetTimerManager() const {
    return *timer_manager;
}

SharedPage::Handler& KernelSystem::GetSharedPageHandler() {
    return *shared_page_handler;
}

const SharedPage::Handler& KernelSystem::GetSharedPageHandler() const {
    return *shared_page_handler;
}

void KernelSystem::AddNamedPort(std::string name, std::shared_ptr<ClientPort> port) {
    named_ports.emplace(std::move(name), std::move(port));
}

} // namespace Kernel
