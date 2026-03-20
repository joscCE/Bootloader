#include <efi.h>



//protocolos

EFI_GUID gEfiLoadedImageProtocolGuid = 
    {0x5B1B31A1,0x9562,0x11d2,{0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}};

EFI_GUID gEfiSimpleFileSystemProtocolGuid = 
    {0x964e5b22,0x6459,0x11d2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}};

EFI_GUID gEfiFileInfoGuid = 
    {0x09576e92,0x6d3f,0x11d2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}};


//funcion principal, recibe el identificador de la imagen y la tabla de servios 

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    SystemTable->ConOut->OutputString( //imprimimos en la termiaal
        SystemTable->ConOut,
        L"UEFI OK\r\n"
    );

    EFI_STATUS status;

    EFI_LOADED_IMAGE *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *File;

    // Obtenemos la loadimage
    status = SystemTable->BootServices->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (void**)&LoadedImage
    );

    if (EFI_ERROR(status)) return status;

    //protocolo para acceder al sistema de archivos
    status = SystemTable->BootServices->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (void**)&FileSystem
    );

    if (EFI_ERROR(status)) return status;

    //abrimos la raiz de los archivos
    status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(status)) return status;

    //abrimos el game.efi
    status = Root->Open(
        Root,
        &File,
        L"\\EFI\\BOOT\\game.efi",
        EFI_FILE_MODE_READ,
        0
    );

    //si no se encontro el archivo
    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"No se encontro game.efi\r\n");
        while (1);
    }

    // obtenemos el tamano del archivo para leerlo completo
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + 200;

    //reservamos la memoria
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

    //leemos el archivo a memoria
    void *Buffer;
    SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        FileSize,
        &Buffer
    );

    File->Read(File, &FileSize, Buffer);

    //CArgamos la imagen a memoria
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

    // Ejecutamos el game
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