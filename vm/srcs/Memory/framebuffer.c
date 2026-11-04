#include<stdint.h>
#include<stdbool.h>
#include<stdlib.h>
#include<stddef.h>
#include<string.h>
#include<stdio.h>

#include<Memory/framebuffer.h>
#include<Memory/memory.h>

framebuffer* framebuffer_init(size_t sizeB) {
    framebuffer* f = malloc(sizeof(framebuffer));
    if (f == NULL) return NULL;

    f->size = sizeB;
    f->mem = calloc(sizeB, sizeof(uint8_t));
    if (f->mem == NULL) {
        free(f);
        return NULL;
    }

    int result = pthread_mutex_init(&f->mutex, NULL);
    if (result != 0) {
        fprintf(stderr, "Unable to initialize framebuffer mutex (error %d).\n", result);
        free(f->mem);
        free(f);
        return NULL;
    }
    return f;
}

uint32_t addr2framebuffer(uint32_t addr) {
    return addr - FRAMEBUFFER_START;
}

int framebuffer_size(const framebuffer* m) {
    return m == NULL ? 0 : (int)m->size;
}

int framebuffer_is_valid_addr(framebuffer* m, uint32_t addr) {
    return m != NULL &&
        addr >= FRAMEBUFFER_START &&
        (size_t)(addr - FRAMEBUFFER_START) < m->size;
}

int framebuffer_lock(framebuffer* m) {
    if (m == NULL) return -1;
    int result = pthread_mutex_lock(&m->mutex);
    if (result != 0) {
        fprintf(stderr, "Unable to lock framebuffer (error %d).\n", result);
        return -1;
    }
    return 0;
}

int framebuffer_unlock(framebuffer* m) {
    if (m == NULL) return -1;
    int result = pthread_mutex_unlock(&m->mutex);
    if (result != 0) {
        fprintf(stderr, "Unable to unlock framebuffer (error %d).\n", result);
        return -1;
    }
    return 0;
}

int framebuffer_write_byte(framebuffer* m, uint32_t addr, uint8_t val) {
    return framebuffer_write_bytes(m, addr, 1, &val);
}

int framebuffer_write_word(framebuffer* m, uint32_t addr, uint16_t val) {
    uint8_t bytes[2] = {(uint8_t)(val >> 8), (uint8_t)val};
    return framebuffer_write_bytes(m, addr, 2, bytes);
}

int framebuffer_write_dword(framebuffer* m, uint32_t addr, uint32_t val) {
    uint8_t bytes[4] = {
        (uint8_t)(val >> 24), (uint8_t)(val >> 16),
        (uint8_t)(val >> 8), (uint8_t)val
    };
    return framebuffer_write_bytes(m, addr, 4, bytes);
}

int framebuffer_write_bytes(framebuffer* m, uint32_t addr, int count, uint8_t* vals) {
    if (m == NULL || vals == NULL || count < 0 ||
        addr < FRAMEBUFFER_START ||
        (size_t)(addr - FRAMEBUFFER_START) > m->size ||
        (size_t)count > m->size - (size_t)(addr - FRAMEBUFFER_START)) {
        return MEM_WRITE_FAILURE;
    }
    if (framebuffer_lock(m) != 0) return MEM_WRITE_FAILURE;
    memcpy(m->mem + addr2framebuffer(addr), vals, (size_t)count);
    int unlock_result = framebuffer_unlock(m);
    return unlock_result == 0 ? MEM_WRITE_SUCCESS : MEM_WRITE_FAILURE;
}

int framebuffer_read_byte(framebuffer* m, uint32_t addr, uint8_t* result) {
    return framebuffer_read_bytes(m, addr, 1, result);
}

int framebuffer_read_word(framebuffer* m, uint32_t addr, uint16_t* result) {
    if (result == NULL) return MEM_READ_FAILURE;
    uint8_t bytes[2];
    int status = framebuffer_read_bytes(m, addr, 2, bytes);
    if (status == MEM_READ_SUCCESS) *result = ((uint16_t)bytes[0] << 8) | bytes[1];
    return status;
}

int framebuffer_read_dword(framebuffer* m, uint32_t addr, uint32_t* result) {
    if (result == NULL) return MEM_READ_FAILURE;
    uint8_t bytes[4];
    int status = framebuffer_read_bytes(m, addr, 4, bytes);
    if (status == MEM_READ_SUCCESS) {
        *result = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
            ((uint32_t)bytes[2] << 8) | bytes[3];
    }
    return status;
}

int framebuffer_read_bytes(framebuffer* m, uint32_t addr, int count, uint8_t* result) {
    if (m == NULL || result == NULL || count < 0 ||
        addr < FRAMEBUFFER_START ||
        (size_t)(addr - FRAMEBUFFER_START) > m->size ||
        (size_t)count > m->size - (size_t)(addr - FRAMEBUFFER_START)) {
        return MEM_READ_FAILURE;
    }
    if (framebuffer_lock(m) != 0) return MEM_READ_FAILURE;
    memcpy(result, m->mem + addr2framebuffer(addr), (size_t)count);
    int unlock_result = framebuffer_unlock(m);
    return unlock_result == 0 ? MEM_READ_SUCCESS : MEM_READ_FAILURE;
}

void framebuffer_reset(framebuffer* m) {
    if (m == NULL || framebuffer_lock(m) != 0) return;
    memset(m->mem, 0, m->size);
    framebuffer_unlock(m);
}

void framebuffer_destroy(framebuffer* f) {
    if (f == NULL) return;
    int result = pthread_mutex_destroy(&f->mutex);
    if (result != 0) {
        fprintf(stderr, "Unable to destroy framebuffer mutex (error %d).\n", result);
        return;
    }
    free(f->mem);
    free(f);
}
