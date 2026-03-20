#include <efi.h>




EFI_GUID gEfiLoadedImageProtocolGuid = 
    {0x5B1B31A1,0x9562,0x11d2,{0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}};

EFI_GUID gEfiSimpleFileSystemProtocolGuid = 
    {0x964e5b22,0x6459,0x11d2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}};

EFI_GUID gEfiFileInfoGuid = 
    {0x09576e92,0x6d3f,0x11d2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}};


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    SystemTable->ConOut->OutputString(
        SystemTable->ConOut,
        L"UEFI OK\r\n"
    );

    EFI_STATUS status;

    EFI_LOADED_IMAGE *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *File;

    // 1. Obtener LoadedImage
    status = SystemTable->BootServices->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (void**)&LoadedImage
    );

    if (EFI_ERROR(status)) return status;

    // 2. Obtener filesystem
    status = SystemTable->BootServices->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (void**)&FileSystem
    );

    if (EFI_ERROR(status)) return status;

    // 3. Abrir volumen
    status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(status)) return status;

    // 4. Abrir game.efi
    status = Root->Open(
        Root,
        &File,
        L"\\EFI\\BOOT\\game.efi",
        EFI_FILE_MODE_READ,
        0
    );

    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"No se encontro game.efi\r\n");
        while (1);
    }

    // 5. Obtener tamaño del archivo
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + 200;

    SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        FileInfoSize,
        (void**)&FileInfo
    );

    File->GetInfo(
        File,
        &gEfiFileInfoGuid,
        &FileInfoSize,
        FileInfo
    );

    UINTN FileSize = FileInfo->FileSize;

    // 6. Leer archivo a memoria
    void *Buffer;
    SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        FileSize,
        &Buffer
    );

    File->Read(File, &FileSize, Buffer);

    // 7. Cargar imagen desde memoria
    EFI_HANDLE GameImage;

    status = SystemTable->BootServices->LoadImage(
        FALSE,
        ImageHandle,
        NULL,
        Buffer,
        FileSize,
        &GameImage
    );

    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Error LoadImage\r\n");
        while (1);
    }

    // 8. Ejecutar
    status = SystemTable->BootServices->StartImage(
        GameImage,
        NULL,
        NULL
    );

    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Error StartImage\r\n");
        while (1);
    }

    while (1);
    return EFI_SUCCESS;
}