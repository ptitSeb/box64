#ifndef __VULKANOVERLAY_H__
#define __VULKANOVERLAY_H__

void* LoadVulkanOverlay(const char* path, int flags);
void* GetSymbolVulkanOverlay(void* g, const char* sym);
int FreeVulkanOverlay(void* g);

#endif //__VULKANOVERLAY_H__