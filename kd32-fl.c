#include "flash.h"

#define BLOCK_SIZE 				1024

typedef struct {
	uint32_t Size;
	uint32_t Address;
} SectorInfo_t;

typedef struct {
	uint16_t AlgoVer;
	uint8_t  Name[128];
	uint16_t Type;
	uint32_t BaseAddr;
	uint32_t TotalSize;
	uint32_t PageSize;
	uint32_t Reserved;
	uint8_t  ErasedVal;
	uint8_t  Padding[3];
	uint32_t TimeoutProg;
	uint32_t TimeoutErase;
	SectorInfo_t SectorInfo[2];
} FlashDevice_t;

extern const FlashDevice_t FlashDevice  __attribute__((section(".constdata")));

const FlashDevice_t FlashDevice = {
	0x0101,
	"KD32F328CB Internal Flash",
	1,
	0x08000000,
	0x00020000,
	0x00000400,
	0x00000000,
	0xFF,
	{ 0x00, 0x00, 0x00},
	500,
	500,
	{
		{ 0x00020000, 0x08000000 },
		{ 0xFFFFFFFF, 0xFFFFFFFF },
	}
};

// --------

int Init(uint32_t Addr, uint32_t Freq, uint32_t Func) __attribute__((section("PrgCode")));
int UnInit(uint32_t Func) __attribute__((section("PrgCode")));
int EraseChip(void) __attribute__((section("PrgCode")));
int EraseSector(uint32_t SectorAddr) __attribute__((section("PrgCode")));
// int ProgramPage(uint32_t DestAddr, uint32_t NumBytes, const uint8_t *pSrcBuff) __attribute__((section("PrgCode")));
int CheckBlank(uint32_t Addr, uint32_t NumBytes, uint8_t BlankValue) __attribute__((section("PrgCode")));

int Init(uint32_t Addr, uint32_t Freq, uint32_t Func)
{  /*1. 设置闪存等待周期（关键参数） */
    // 根据主频计算等待周期（参考数据手册第3.2节）
    // 示例：72MHz时需2个等待周期
    FLASH_ACR = (FLASH_ACR & ~FLASH_ACR_LATENCY_MASK) | 0x02;

    /* 2. 使能预取缓冲区（提升性能） */
    FLASH_ACR |= FLASH_ACR_PRFTEN;

    /* 3. 删除原FLASH_MASK相关操作（新芯片无此寄存器） */

    /* 4. 验证配置是否生效 */
    if ((FLASH_ACR & (FLASH_ACR_LATENCY_MASK | FLASH_ACR_PRFTEN))
        != (0x02 | FLASH_ACR_PRFTEN)) {
        return 1; // 配置失败
    }

	return 0;
}

int UnInit(uint32_t Func)
{
	// restore the MCU

	return 0;
}

int EraseChip(void)
{
	for (uint32_t i = 0; i < 0x00020000; i += 1024) { // 128KB 范围
        int ret = EraseSector(i);
        if (ret) return ret;
    }
    return 0;

}


int EraseSector(uint32_t SectorAddr)
{
	// 修改对齐检查为 1024 字节
    if (SectorAddr % 1024 || SectorAddr >= 0x08020000) {
        return 1;
    }

    FLASH_UNLOCK();
    FLASH_ERASE_PAGE(SectorAddr);
    FLASH_LOCK();

    return 0;
}

// 编程函数适配
int ProgramPage(uint32_t DestAddr, const uint8_t *pBuffer, uint32_t NumBytes)
{
	if (DestAddr % 1024 || DestAddr >= 0x08020000 ||
        NumBytes % 1024 || (DestAddr + NumBytes) > 0x08020000) {
        return 1;
    }

    if (DestAddr % 4 || NumBytes % 4) {  // 必须按字对齐
        return 1;
    }

    FLASH_UNLOCK();
    for (uint32_t i = 0; i < NumBytes; i += 4) {
        uint32_t word = *((uint32_t*)(pBuffer + i));
        FLASH_PROGRAM_WORD(DestAddr + i, word);
    }
    FLASH_LOCK();

    return 0;
}


void FlashErasePage(uint32_t PageAddress)
{
    FLASH_CR |= FLASH_CR_PER;     // 页擦除模式
    FLASH_AR = PageAddress;       // 设置页地址
    FLASH_CR |= FLASH_CR_STRT;    // 启动擦除
    while (FLASH_SR & FLASH_SR_BSY); // 等待完成
    FLASH_CR &= ~FLASH_CR_PER;    // 清除模式
}

void FlashProgramWord(uint32_t Address, uint32_t Data)
{
    FLASH_CR |= FLASH_CR_PG;      // 使能编程模式
    *((volatile uint32_t*)Address) = Data; // 写入数据
    while (FLASH_SR & FLASH_SR_BSY); // 等待完成
    FLASH_CR &= ~FLASH_CR_PG;     // 清除模式
}
