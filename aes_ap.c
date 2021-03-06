#include "golb.h"
#include <sys/stat.h>
#include <sys/sysctl.h>

#define PMGR_SZ (0x100000)
#define IO_BASE (0x200000000ULL)

#define OP_IV (2U)
#define OP_KEY (1U)
#define OP_DATA (5U)
#define OP_FLAG (8U)
#define KEY_LEN_128 (0U)
#define KEY_LEN_192 (1U)
#define KEY_LEN_256 (2U)
#define AES_CMD_ENC (0U)
#define AES_CMD_DEC (1U)
#define AES_CMD_ECB (0U)
#define AES_CMD_CBC (16U)
#define AES_BLOCK_SZ (16)
#define CMD_OP_SHIFT (28U)
#define BLOCK_MODE_ECB (0U)
#define BLOCK_MODE_CBC (1U)
#define AES_KEY_SZ_128 (0U)
#define KEY_SELECT_UID1 (1U)
#define PMGR_PS_RUN_MAX (15U)
#define AES_CMD_DIR_MASK (15U)
#define KEY_SELECT_GID_AP_1 (2U)
#define KEY_SELECT_GID_AP_2 (3U)
#define TXT_IN_STS_RDY (1U << 0U)
#define AES_CMD_MODE_MASK (0xF0U)
#define AES_KEY_TYPE_UID0 (0x100U)
#define AES_KEY_TYPE_GID0 (0x200U)
#define AES_KEY_TYPE_GID1 (0x201U)
#define AES_KEY_TYPE_MASK (0xFFFU)
#define CMD_DATA_CMD_LEN_SHIFT (0U)
#define AES_KEY_SZ_192 (0x10000000U)
#define AES_KEY_SZ_256 (0x20000000U)
#define PMGR_PS_ACTUAL_PS_MASK (15U)
#define PMGR_PS_MANUAL_PS_MASK (15U)
#define AES_BLK_CTRL_STOP_UMASK (2U)
#define PMGR_PS_ACTUAL_PS_SHIFT (4U)
#define AES_KEY_SZ_MASK (0xF0000000U)
#define IV_IN_CTRL_VAL_SET (1U << 0U)
#define CMD_FLAG_SEND_INT_SHIFT (27U)
#define AES_BLK_CTRL_START_UMASK (1U)
#define KEY_IN_CTRL_LEN_128 (0U << 6U)
#define KEY_IN_CTRL_LEN_192 (1U << 6U)
#define KEY_IN_CTRL_LEN_256 (2U << 6U)
#define KEY_IN_CTRL_VAL_SET (1U << 0U)
#define TXT_IN_CTRL_VAL_SET (1U << 0U)
#define TXT_OUT_STS_VAL_SET (1U << 0U)
#define CMD_FLAG_STOP_CMDS_SHIFT (26U)
#define KEY_IN_CTRL_MOD_ECB (0U << 13U)
#define KEY_IN_CTRL_MOD_CBC (1U << 13U)
#define KEY_IN_CTRL_DIR_DEC (0U << 12U)
#define KEY_IN_CTRL_DIR_ENC (1U << 12U)
#define KEY_IN_CTRL_SEL_UID1 (1U << 4U)
#define KEY_IN_CTRL_SEL_GID0 (2U << 4U)
#define KEY_IN_CTRL_SEL_GID1 (3U << 4U)
#define AES_AP_SZ (vm_kernel_page_size)
#define CMD_KEY_CMD_KEY_LEN_SHIFT (22U)
#define CMD_KEY_CMD_ENCRYPT_SHIFT (20U)
#define CMD_DATA_CMD_LEN_MASK (0xFFFFFFU)
#define CMD_KEY_CMD_KEY_SELECT_SHIFT (24U)
#define CMD_KEY_CMD_BLOCK_MODE_SHIFT (16U)
#define CMD_DATA_UPPER_ADDR_DST_SHIFT (0U)
#define CMD_DATA_UPPER_ADDR_SRC_SHIFT (16U)
#define CMD_DATA_UPPER_ADDR_DST_MASK (0xFFU)
#define CMD_DATA_UPPER_ADDR_SRC_MASK (0xFFU)
#define AES_BLK_INT_STATUS_FLAG_CMD_UMASK (32U)
#define PMGR_BASE_ADDR (IO_BASE + pmgr_base_off)
#define AES_AP_BASE_ADDR (IO_BASE + aes_ap_base_off)
#define rAES_CTRL (*(volatile uint32_t *)(aes_ap_virt_base + 0x8))
#define rAES_AP_DIS (*(volatile uint32_t *)(aes_ap_virt_base + 0x4))
#define rAES_CMD_FIFO (*(volatile uint32_t *)(aes_ap_virt_base + 0x200))
#define rAES_AP_IV_IN0 (*(volatile uint32_t *)(aes_ap_virt_base + 0x100))
#define rAES_AP_IV_IN1 (*(volatile uint32_t *)(aes_ap_virt_base + 0x104))
#define rAES_AP_IV_IN2 (*(volatile uint32_t *)(aes_ap_virt_base + 0x108))
#define rAES_AP_IV_IN3 (*(volatile uint32_t *)(aes_ap_virt_base + 0x10C))
#define rAES_AP_TXT_IN0 (*(volatile uint32_t *)(aes_ap_virt_base + 0x40))
#define rAES_AP_TXT_IN1 (*(volatile uint32_t *)(aes_ap_virt_base + 0x44))
#define rAES_AP_TXT_IN2 (*(volatile uint32_t *)(aes_ap_virt_base + 0x48))
#define rAES_AP_TXT_IN3 (*(volatile uint32_t *)(aes_ap_virt_base + 0x4C))
#define rAES_INT_STATUS (*(volatile uint32_t *)(aes_ap_virt_base + 0x18))
#define rAES_AP_TXT_OUT0 (*(volatile uint32_t *)(aes_ap_virt_base + 0x80))
#define rAES_AP_TXT_OUT1 (*(volatile uint32_t *)(aes_ap_virt_base + 0x84))
#define rAES_AP_TXT_OUT2 (*(volatile uint32_t *)(aes_ap_virt_base + 0x88))
#define rAES_AP_TXT_OUT3 (*(volatile uint32_t *)(aes_ap_virt_base + 0x8C))
#define rAES_AP_TXT_IN_STS (*(volatile uint32_t *)(aes_ap_virt_base + 0xC))
#define rAES_AP_IV_IN_CTRL (*(volatile uint32_t *)(aes_ap_virt_base + 0xE0))
#define rAES_AP_TXT_IN_CTRL (*(volatile uint32_t *)(aes_ap_virt_base + 0x8))
#define rAES_AP_KEY_IN_CTRL (*(volatile uint32_t *)(aes_ap_virt_base + 0x90))
#define rAES_AP_TXT_OUT_STS (*(volatile uint32_t *)(aes_ap_virt_base + 0x50))
#define rPMGR_AES0_PS (*(volatile uint32_t *)(pmgr_virt_base + pmgr_aes0_ps_off))

typedef struct {
	uint32_t cmd, iv[4];
} cmd_iv_t;

typedef struct {
	uint32_t cmd, key[8];
} cmd_key_t;

typedef struct {
	uint32_t key_id, key[4], val[4];
} key_seed_t;

typedef struct {
	uint32_t cmd, upper_addr, src_addr, dst_addr;
} cmd_data_t;

static bool aes_ap_v2;
static golb_ctx_t pmgr_ctx, aes_ap_ctx;
static kaddr_t aes_ap_base_off, pmgr_base_off, aes_ap_virt_base, pmgr_virt_base, pmgr_aes0_ps_off;

static key_seed_t uid_key_seeds[] = {
	{ 0x839, { 0xC55BB624, 0xDCDCDD8F, 0x6C8B5498, 0x4D84E73E }, { 0 } },
	{ 0x83A, { 0xDBAB10CB, 0x63ECC98A, 0xB4C228DB, 0x060ED6A9 }, { 0 } },
	{ 0x83B, { 0x87D0A77D, 0x171EFE90, 0xB83E2DC6, 0x2D94D81F }, { 0 } },
	{ 0x83C, { 0xD34AC2B2, 0xD84BF05D, 0x547433A0, 0x644EE6C4 }, { 0 } },
	{ 0x899, { 0xB5FCE8D1, 0x8DBF3739, 0xD14CC7EF, 0xB0D4F1D0 }, { 0 } },
	{ 0x89B, { 0x67993E18, 0x543CB06B, 0xF568A46F, 0x49BD0C1C }, { 0 } },
	{ 0x89C, { 0x7140B400, 0xCFF3A1A8, 0xFF9B2FD9, 0xFCDD75FB }, { 0 } },
	{ 0x89D, { 0xD8C29F34, 0x2C8AFBA6, 0xB47C0329, 0xAD23DAAC }, { 0 } },
	{ 0x8A0, { 0xC599B4D1, 0xDC3CA139, 0xD19BA498, 0xB0DD0C3E }, { 0 } },
	{ 0x8A3, { 0x65418256, 0xCDE05165, 0x4CF86FF5, 0xEF791AC1 }, { 0 } },
	{ 0x8A4, { 0xDFF7310C, 0x034D9281, 0xFA37B48C, 0xC9F76003 }, { 0 } }
};

static kern_return_t
init_arm_globals(void) {
	int cpufamily = CPUFAMILY_UNKNOWN;
	size_t len = sizeof(cpufamily);

	if(sysctlbyname("hw.cpufamily", &cpufamily, &len, NULL, 0) == 0) {
		switch(cpufamily) {
			case CPUFAMILY_ARM_CYCLONE:
				pmgr_base_off = 0xE000000;
				pmgr_aes0_ps_off = 0x20100;
				aes_ap_base_off = 0xA108000;
				return KERN_SUCCESS;
			case CPUFAMILY_ARM_TYPHOON:
				pmgr_base_off = 0xE000000;
				pmgr_aes0_ps_off = 0x201E8;
				aes_ap_base_off = 0xA108000;
				return KERN_SUCCESS;
			case CPUFAMILY_ARM_TWISTER:
				aes_ap_v2 = true;
				pmgr_base_off = 0xE000000;
				pmgr_aes0_ps_off = 0x80210;
				aes_ap_base_off = 0xA108000;
				return KERN_SUCCESS;
			case CPUFAMILY_ARM_HURRICANE:
				aes_ap_v2 = true;
				pmgr_base_off = 0xE000000;
				pmgr_aes0_ps_off = 0x80220;
				aes_ap_base_off = 0xA108000;
				return KERN_SUCCESS;
			case CPUFAMILY_ARM_MONSOON_MISTRAL:
				aes_ap_v2 = true;
				pmgr_base_off = 0x32000000;
				pmgr_aes0_ps_off = 0x80240;
				aes_ap_base_off = 0x2E008000;
				return KERN_SUCCESS;
#if 0
			case CPUFAMILY_ARM_VORTEX_TEMPEST:
				aes_ap_v2 = true;
				pmgr_base_off = 0x3B000000;
				pmgr_aes0_ps_off = 0x80220;
				aes_ap_base_off = 0x35008000;
				return KERN_SUCCESS;
			case CPUFAMILY_ARM_LIGHTNING_THUNDER:
				aes_ap_v2 = true;
				pmgr_base_off = 0x3B000000;
				pmgr_aes0_ps_off = 0x801D0;
				aes_ap_base_off = 0x35008000;
				return KERN_SUCCESS;
#endif
			default:
				break;
		}
	}
	return KERN_FAILURE;
}

static void
aes_ap_term(void) {
	golb_unmap(aes_ap_ctx);
	mach_vm_deallocate(mach_task_self(), aes_ap_virt_base, AES_AP_SZ);
	golb_unmap(pmgr_ctx);
	mach_vm_deallocate(mach_task_self(), pmgr_virt_base, PMGR_SZ);
}

static kern_return_t
aes_ap_init(void) {
	if(mach_vm_allocate(mach_task_self(), &aes_ap_virt_base, AES_AP_SZ, VM_FLAGS_ANYWHERE) == KERN_SUCCESS) {
		printf("aes_ap_virt_base: " KADDR_FMT "\n", aes_ap_virt_base);
		if(golb_map(&aes_ap_ctx, aes_ap_virt_base, AES_AP_BASE_ADDR, AES_AP_SZ, VM_PROT_READ | VM_PROT_WRITE) == KERN_SUCCESS) {
			if(mach_vm_allocate(mach_task_self(), &pmgr_virt_base, PMGR_SZ, VM_FLAGS_ANYWHERE) == KERN_SUCCESS) {
				printf("pmgr_virt_base: " KADDR_FMT "\n", pmgr_virt_base);
				if(golb_map(&pmgr_ctx, pmgr_virt_base, PMGR_BASE_ADDR, PMGR_SZ, VM_PROT_READ | VM_PROT_WRITE) == KERN_SUCCESS) {
					return KERN_SUCCESS;
				}
				mach_vm_deallocate(mach_task_self(), pmgr_virt_base, PMGR_SZ);
			}
			golb_unmap(aes_ap_ctx);
		}
		mach_vm_deallocate(mach_task_self(), aes_ap_virt_base, AES_AP_SZ);
	}
	return KERN_FAILURE;
}

static kern_return_t
aes_ap_v1_cmd(uint32_t cmd, const void *src, void *dst, size_t len, uint32_t opts) {
	uint32_t *local_dst = dst, key_in_ctrl = 0;
	kern_return_t ret = KERN_FAILURE;
	const uint32_t *local_src = src;
	size_t i;

	if(len == 0 || (len % AES_BLOCK_SZ) != 0) {
		return KERN_FAILURE;
	}
	switch(cmd & AES_CMD_MODE_MASK) {
		case AES_CMD_ECB:
			key_in_ctrl |= KEY_IN_CTRL_MOD_ECB;
			break;
		case AES_CMD_CBC:
			key_in_ctrl |= KEY_IN_CTRL_MOD_CBC;
			break;
		default:
			return ret;
	}
	switch(cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC:
			key_in_ctrl |= KEY_IN_CTRL_DIR_ENC;
			break;
		case AES_CMD_DEC:
			key_in_ctrl |= KEY_IN_CTRL_DIR_DEC;
			break;
		default:
			return ret;
	}
	switch(opts & AES_KEY_SZ_MASK) {
		case AES_KEY_SZ_128:
			key_in_ctrl |= KEY_IN_CTRL_LEN_128;
			break;
		case AES_KEY_SZ_192:
			key_in_ctrl |= KEY_IN_CTRL_LEN_192;
			break;
		case AES_KEY_SZ_256:
			key_in_ctrl |= KEY_IN_CTRL_LEN_256;
			break;
		default:
			return ret;
	}
	switch(opts & AES_KEY_TYPE_MASK) {
		case AES_KEY_TYPE_UID0:
			key_in_ctrl |= KEY_IN_CTRL_SEL_UID1;
			break;
		case AES_KEY_TYPE_GID0:
			key_in_ctrl |= KEY_IN_CTRL_SEL_GID0;
			break;
		case AES_KEY_TYPE_GID1:
			key_in_ctrl |= KEY_IN_CTRL_SEL_GID1;
			break;
		default:
			return ret;
	}
	rPMGR_AES0_PS |= PMGR_PS_RUN_MAX;
	while((rPMGR_AES0_PS & PMGR_PS_MANUAL_PS_MASK) != ((rPMGR_AES0_PS >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK)) {}
	if((~rAES_AP_DIS & (opts & AES_KEY_TYPE_MASK)) != 0) {
		printf("old_iv: 0x%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "\n", rAES_AP_IV_IN0, rAES_AP_IV_IN1, rAES_AP_IV_IN2, rAES_AP_IV_IN3);
		printf("old_in: 0x%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "\n", rAES_AP_TXT_IN0, rAES_AP_TXT_IN1, rAES_AP_TXT_IN2, rAES_AP_TXT_IN3);
		printf("old_out: 0x%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "\n", rAES_AP_TXT_OUT0, rAES_AP_TXT_OUT1, rAES_AP_TXT_OUT2, rAES_AP_TXT_OUT3);
		rAES_AP_KEY_IN_CTRL = key_in_ctrl | KEY_IN_CTRL_VAL_SET;
		rAES_AP_IV_IN0 = rAES_AP_IV_IN1 = rAES_AP_IV_IN2 = rAES_AP_IV_IN3 = 0;
		rAES_AP_IV_IN_CTRL = IV_IN_CTRL_VAL_SET;
		for(i = 0; i < len / AES_BLOCK_SZ; ++i) {
			while((~rAES_AP_TXT_IN_STS & TXT_IN_STS_RDY) != 0) {}
			rAES_AP_TXT_IN0 = local_src[i];
			rAES_AP_TXT_IN1 = local_src[i + 1];
			rAES_AP_TXT_IN2 = local_src[i + 2];
			rAES_AP_TXT_IN3 = local_src[i + 3];
			rAES_AP_TXT_IN_CTRL = TXT_IN_CTRL_VAL_SET;
			while((~rAES_AP_TXT_OUT_STS & TXT_OUT_STS_VAL_SET) != 0) {}
			local_dst[i] = rAES_AP_TXT_OUT0;
			local_dst[i + 1] = rAES_AP_TXT_OUT1;
			local_dst[i + 2] = rAES_AP_TXT_OUT2;
			local_dst[i + 3] = rAES_AP_TXT_OUT3;
		}
		ret = KERN_SUCCESS;
	}
	rPMGR_AES0_PS &= ~PMGR_PS_RUN_MAX;
	while((rPMGR_AES0_PS & PMGR_PS_MANUAL_PS_MASK) != ((rPMGR_AES0_PS >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK)) {}
	return ret;
}

static void
push_cmds(const uint32_t *cmd, size_t len) {
	size_t i;

	for(i = 0; i < len / sizeof(*cmd); ++i) {
		rAES_CMD_FIFO = cmd[i];
	}
}

static kern_return_t
aes_ap_v2_cmd(uint32_t cmd, kaddr_t phys_src, kaddr_t phys_dst, size_t len, uint32_t opts) {
	cmd_data_t data;
	cmd_key_t ckey;
	uint32_t flag;
	cmd_iv_t civ;

	if(len == 0 || (len % AES_BLOCK_SZ) != 0) {
		return KERN_FAILURE;
	}
	ckey.cmd = OP_KEY << CMD_OP_SHIFT;
	switch(cmd & AES_CMD_MODE_MASK) {
		case AES_CMD_ECB:
			ckey.cmd |= BLOCK_MODE_ECB << CMD_KEY_CMD_BLOCK_MODE_SHIFT;
			break;
		case AES_CMD_CBC:
			ckey.cmd |= BLOCK_MODE_CBC << CMD_KEY_CMD_BLOCK_MODE_SHIFT;
			break;
		default:
			return KERN_FAILURE;
	}
	switch(cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC:
			ckey.cmd |= 1U << CMD_KEY_CMD_ENCRYPT_SHIFT;
			break;
		case AES_CMD_DEC:
			ckey.cmd |= 0U << CMD_KEY_CMD_ENCRYPT_SHIFT;
			break;
		default:
			return KERN_FAILURE;
	}
	switch(opts & AES_KEY_SZ_MASK) {
		case AES_KEY_SZ_128:
			ckey.cmd |= KEY_LEN_128 << CMD_KEY_CMD_KEY_LEN_SHIFT;
			break;
		case AES_KEY_SZ_192:
			ckey.cmd |= KEY_LEN_192 << CMD_KEY_CMD_KEY_LEN_SHIFT;
			break;
		case AES_KEY_SZ_256:
			ckey.cmd |= KEY_LEN_256 << CMD_KEY_CMD_KEY_LEN_SHIFT;
			break;
		default:
			return KERN_FAILURE;
	}
	switch(opts & AES_KEY_TYPE_MASK) {
		case AES_KEY_TYPE_UID0:
			ckey.cmd |= KEY_SELECT_UID1 << CMD_KEY_CMD_KEY_SELECT_SHIFT;
			break;
		case AES_KEY_TYPE_GID0:
			ckey.cmd |= KEY_SELECT_GID_AP_1 << CMD_KEY_CMD_KEY_SELECT_SHIFT;
			break;
		case AES_KEY_TYPE_GID1:
			ckey.cmd |= KEY_SELECT_GID_AP_2 << CMD_KEY_CMD_KEY_SELECT_SHIFT;
			break;
		default:
			return KERN_FAILURE;
	}
	rPMGR_AES0_PS |= PMGR_PS_RUN_MAX;
	while((rPMGR_AES0_PS & PMGR_PS_MANUAL_PS_MASK) != ((rPMGR_AES0_PS >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK)) {}
	rAES_INT_STATUS = AES_BLK_INT_STATUS_FLAG_CMD_UMASK;
	rAES_CTRL = AES_BLK_CTRL_START_UMASK;
	push_cmds(&ckey.cmd, sizeof(ckey.cmd));
	civ.cmd = OP_IV << CMD_OP_SHIFT;
	memset(&civ.iv, '\0', sizeof(civ.iv));
	push_cmds(&civ.cmd, sizeof(civ));
	data.cmd = (OP_DATA << CMD_OP_SHIFT) | ((uint32_t)(len & CMD_DATA_CMD_LEN_MASK) << CMD_DATA_CMD_LEN_SHIFT);
	data.upper_addr = ((uint32_t)(phys_src >> 32U) & CMD_DATA_UPPER_ADDR_SRC_MASK) << CMD_DATA_UPPER_ADDR_SRC_SHIFT;
	data.upper_addr |= ((uint32_t)(phys_dst >> 32U) & CMD_DATA_UPPER_ADDR_DST_MASK) << CMD_DATA_UPPER_ADDR_DST_SHIFT;
	data.src_addr = (uint32_t)phys_src;
	data.dst_addr = (uint32_t)phys_dst;
	push_cmds(&data.cmd, sizeof(data));
	flag = (OP_FLAG << CMD_OP_SHIFT) | (1U << CMD_FLAG_SEND_INT_SHIFT) | (1U << CMD_FLAG_STOP_CMDS_SHIFT);
	push_cmds(&flag, sizeof(flag));
	while((rAES_INT_STATUS & AES_BLK_INT_STATUS_FLAG_CMD_UMASK) == 0) {}
	rAES_INT_STATUS = AES_BLK_INT_STATUS_FLAG_CMD_UMASK;
	rAES_CTRL = AES_BLK_CTRL_STOP_UMASK;
	if(pmgr_aes0_ps_off != 0x80240) {
		rPMGR_AES0_PS &= ~PMGR_PS_RUN_MAX;
		while((rPMGR_AES0_PS & PMGR_PS_MANUAL_PS_MASK) != ((rPMGR_AES0_PS >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK)) {}
	}
	return KERN_SUCCESS;
}

static kern_return_t
aes_ap_cmd(uint32_t cmd, const void *src, void *dst, size_t len, uint32_t opts) {
	kaddr_t virt_src = (kaddr_t)src, virt_dst = (kaddr_t)dst, phys_src, phys_dst;
	vm_machine_attribute_val_t mattr_val = MATTR_VAL_CACHE_FLUSH;
	size_t aes_len;

	if(aes_ap_v2) {
		do {
			if((phys_src = golb_find_phys(virt_src)) == 0) {
				return KERN_FAILURE;
			}
			printf("phys_src: " KADDR_FMT "\n", phys_src);
			if((phys_dst = golb_find_phys(virt_dst)) == 0) {
				return KERN_FAILURE;
			}
			printf("phys_dst: " KADDR_FMT "\n", phys_dst);
			aes_len = MIN(len, MIN(vm_kernel_page_size - (phys_src & vm_kernel_page_mask), vm_kernel_page_size - (phys_dst & vm_kernel_page_mask)));
			if(aes_ap_v2_cmd(cmd, phys_src, phys_dst, aes_len, opts) != KERN_SUCCESS || mach_vm_machine_attribute(mach_task_self(), virt_dst, aes_len, MATTR_CACHE, &mattr_val) != KERN_SUCCESS) {
				return KERN_FAILURE;
			}
			virt_src += aes_len;
			virt_dst += aes_len;
			len -= aes_len;
		} while(len != 0);
		return KERN_SUCCESS;
	}
	return aes_ap_v1_cmd(cmd, src, dst, len, opts);
}

static void
aes_ap_test(void) {
	size_t i;

	for(i = 0; i < sizeof(uid_key_seeds) / sizeof(*uid_key_seeds); ++i) {
		if(aes_ap_cmd(AES_CMD_CBC | AES_CMD_ENC, uid_key_seeds[i].key, uid_key_seeds[i].val, sizeof(uid_key_seeds[i].key), AES_KEY_SZ_256 | AES_KEY_TYPE_UID0) == KERN_SUCCESS) {
			printf("key_id: 0x%" PRIX32 ", val: 0x%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "\n", uid_key_seeds[i].key_id, uid_key_seeds[i].val[0], uid_key_seeds[i].val[1], uid_key_seeds[i].val[2], uid_key_seeds[i].val[3]);
		}
	}
}

static kern_return_t
aes_ap_file(const char *dir, const char *key_type, const char *in_filename, const char *out_filename) {
	uint32_t cmd = AES_CMD_CBC, opts = AES_KEY_SZ_256;
	kern_return_t ret = KERN_FAILURE;
	struct stat stat_buf;
	int in_fd, out_fd;
	size_t len;
	void *buf;

	if(strcmp(dir, "enc") == 0) {
		cmd |= AES_CMD_ENC;
	} else if(strcmp(dir, "dec") == 0) {
		cmd |= AES_CMD_DEC;
	} else {
		return ret;
	}
	if(strcmp(key_type, "UID0") == 0) {
		opts |= AES_KEY_TYPE_UID0;
	} else if(strcmp(key_type, "GID0") == 0) {
		opts |= AES_KEY_TYPE_GID0;
	} else if(strcmp(key_type, "GID1") == 0) {
		opts |= AES_KEY_TYPE_GID1;
	} else {
		return ret;
	}
	if((in_fd = open(in_filename, O_RDONLY | O_CLOEXEC)) != -1) {
		if(fstat(in_fd, &stat_buf) != -1 && stat_buf.st_size > 0) {
			len = (size_t)stat_buf.st_size;
			if((len % AES_BLOCK_SZ) == 0 && (buf = malloc(len)) != NULL) {
				if(read(in_fd, buf, len) != -1 && aes_ap_cmd(cmd, buf, buf, len, opts) == KERN_SUCCESS && (out_fd = open(out_filename, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, S_IROTH | S_IRGRP | S_IWUSR | S_IRUSR)) != -1) {
					if(write(out_fd, buf, len) != -1) {
						ret = KERN_SUCCESS;
					}
					close(out_fd);
				}
				free(buf);
			}
		}
		close(in_fd);
	}
	return ret;
}

int
main(int argc, char **argv) {
	if(argc >= 2 && argc < 5) {
		printf("Usage: %s [enc/dec UID0/GID0/GID1 in out]\n", argv[0]);
	} else if(init_arm_globals() == KERN_SUCCESS) {
		printf("pmgr_base_off: " KADDR_FMT ", aes_ap_base_off: " KADDR_FMT ", pmgr_aes0_ps_off: " KADDR_FMT "\n", pmgr_base_off, aes_ap_base_off, pmgr_aes0_ps_off);
		if(golb_init() == KERN_SUCCESS) {
			if(aes_ap_init() == KERN_SUCCESS) {
				if(argc == 5) {
					printf("aes_ap_file: 0x%" PRIX32 "\n", aes_ap_file(argv[1], argv[2], argv[3], argv[4]));
				} else {
					aes_ap_test();
				}
				aes_ap_term();
			}
			golb_term();
		}
	}
}
