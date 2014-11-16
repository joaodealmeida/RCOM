#include "manipulation.h"

/* --- Stuffing --- */

unsigned char *Stuffing(unsigned char *Destuffed, unsigned short DestuffedSize, unsigned short *StuffedSize)
{
        int i, j;
        unsigned char *Stuffed = malloc(DestuffedSize * 2 * sizeof(unsigned char));

        for (i = 0, j = 0; i < DestuffedSize; i++)
            if (Destuffed[i] == 0x7e && i != 0 && i != DestuffedSize - 1)
            {
                Stuffed[j++] = 0x7d;
                Stuffed[j++] = 0x5e;
            }
            else if (Destuffed[i] == 0x7d && i != 0 && i != DestuffedSize - 1)
            {
                Stuffed[j++] = 0x7d;
                Stuffed[j++] = 0x5d;
            }
            else
                Stuffed[j++] = Destuffed[i];

        (*StuffedSize) = j;
        Stuffed = (unsigned char *) realloc (Stuffed, *StuffedSize * sizeof(unsigned char));

        return Stuffed;
}

/* --- Destuffing --- */

unsigned char *Destuffing(unsigned char *Stuffed, unsigned short StuffedSize, unsigned short *DestuffedSize)
{
        int i, j;
        unsigned char *Destuffed = malloc(StuffedSize * sizeof(unsigned char));

        for (i = 0, j = 0; i < StuffedSize; i++, j++)
            if (Stuffed[i] == 0x7d)
                if (Stuffed[i + 1] == 0x5e && i != 0 && i != StuffedSize - 1)
                {
                    Destuffed[j] = 0x7e;
                    i++;
                }
                else if (Stuffed[i + 1] == 0x5d && i != 0 && i != StuffedSize - 1)
                {
                    Destuffed[j] = 0x7d;
                    i++;
                }
                else
                    Destuffed[j] = 0x7d;
            else
                Destuffed[j] = Stuffed[i];

        (*DestuffedSize) = j;
        Destuffed = (unsigned char *) realloc (Destuffed, *DestuffedSize * sizeof(unsigned char));

        return Destuffed;
}

/* --- CreateDataPacket --- */

unsigned char *CreateDataPacket (DataStruct DataPacket, unsigned short *PacketSize, unsigned short CntrlField, unsigned short *BCC2)
{
    unsigned short L2 = DataPacket.DataSize / 256, L1 = DataPacket.DataSize % 256, i, counter = 0;

    *PacketSize = 4 + DataPacket.DataSize;

    unsigned char *DataString = malloc (sizeof(char) * (*PacketSize));

    DataString[counter] = (unsigned char) CntrlField;
    *BCC2 = DataString[counter];
    counter++;

    DataString[counter] = (unsigned char) DataPacket.SeqNumber;
    *BCC2 ^= DataString[counter];
    counter++;

    DataString[counter] = (unsigned char) L2;
    *BCC2 ^= DataString[counter];
    counter++;

    DataString[counter] = (unsigned char) L1;
    *BCC2 ^= DataString[counter];
    counter++;

    for (i = 0; i < DataPacket.DataSize; i++)
    {
        DataString[counter + i] = DataPacket.Data[i];
        *BCC2 ^= DataString[counter + i];
    }
    counter += DataPacket.DataSize;

    return DataString ;
}

/* --- ReadDataPacket --- */

signed short ReadDataPacket (DataStruct *DataPacket, unsigned char *DataString, unsigned short PacketSize)
{
    signed short BCC2;
    unsigned short counter = 0, i;

    if (!DataString[counter])
        return -1;
    else
    {
        BCC2 = DataString[counter];
        counter++;

        (*DataPacket).SeqNumber = DataString[counter];
        printf ("0. SeqNumber: %lu\n", DataString[counter]);
        printf ("SeqNumber: %lu\n", (*DataPacket).SeqNumber);
        BCC2 ^= DataString[counter];
        counter++;

        printf ("L2: %x\nL1: %x\n", DataString[counter], DataString[counter + 1]);


        //(*DataPacket).DataSize = 256 * atoi(DataString[counter]) + atoi(DataString[counter + 1]);
        (*DataPacket).DataSize = 100;
        if( (*DataPacket).SeqNumber == 110)
            (*DataPacket).DataSize = 68;
         // <---------------
        printf ("DataSize: %lu\n", (*DataPacket).DataSize);
        BCC2 ^= DataString[counter];
        BCC2 ^= DataString[counter + 1];
        counter += 2;

        (*DataPacket).Data = (unsigned char*) malloc (sizeof(char) * (*DataPacket).DataSize);

        for (i = 0; i < (*DataPacket).DataSize; i++)
        {
            (*DataPacket).Data[i] = DataString[counter + i];
            BCC2 ^= DataString[counter + i];
        }

    }

    return BCC2;
}

/* --- CreateCntrlPacket --- */

unsigned char *CreateCntrlPacket (CntrlStruct CntrlPacket, unsigned short *PacketSize, unsigned short CntrlField, unsigned short *BCC2)
{
    unsigned short i, counter = 0;

    *PacketSize = 5 + CntrlPacket.Length[0] + CntrlPacket.Length[1];

    printf ("PacketSize: %hu\n", *PacketSize);

    unsigned char *CntrlString = malloc (sizeof(char) * (*PacketSize));

    CntrlString[counter] = (unsigned char) CntrlField;
    *BCC2 = CntrlString[counter];
    counter++;

    CntrlString[counter] = (unsigned char) CntrlPacket.Type[0];
    *BCC2 ^= CntrlString[counter];
    counter++;

    CntrlString[counter] = (unsigned char) CntrlPacket.Length[0];
    *BCC2 ^= CntrlString[counter];
    counter++;

    for (i = 0; i < CntrlPacket.Length[0]; i++)
    {
        CntrlString[counter + i] = CntrlPacket.FileSize[i];
        *BCC2 ^= CntrlString[counter + i];
    }
    counter += CntrlPacket.Length[0];

    CntrlString[counter] = (unsigned char) CntrlPacket.Type[1];
    *BCC2 ^= CntrlString[counter];
    counter++;

    CntrlString[counter] = (unsigned char) CntrlPacket.Length[1];
    *BCC2 ^= CntrlString[counter];
    counter++;

    for (i = 0; i < CntrlPacket.Length[1]; i++)
    {
        CntrlString[counter + i] = CntrlPacket.Filename[i];
        *BCC2 ^= CntrlString[counter + i];
    }
    counter += CntrlPacket.Length[1];

    return CntrlString;
}

/* --- ReadCntrlPacket --- */

signed short ReadCntrlPacket (CntrlStruct *CntrlPacket, unsigned char *CntrlString, unsigned short PacketSize)
{
    signed short BCC2;
    unsigned short counter = 0, i;

    if (CntrlString[counter] != 2 && CntrlString[counter] != 3)
        return -1;
    else
    {
        printf ("ReadCntrlPacket!\n");


        BCC2 = CntrlString[counter];
        counter++;

        (*CntrlPacket).Type[0] = CntrlString[counter];
        BCC2 ^= CntrlString[counter];
        counter++;

        (*CntrlPacket).Length[0] = CntrlString[counter];
        BCC2 ^= CntrlString[counter];
        counter++;        

        (*CntrlPacket).FileSize = (unsigned char*) malloc (sizeof(char) * (*CntrlPacket).Length[0]);

        for (i = 0; i < (*CntrlPacket).Length[0]; i++)
        {
            (*CntrlPacket).FileSize[i] = CntrlString[counter + i];
            BCC2 ^= CntrlString[counter + i];
        }
        counter += (*CntrlPacket).Length[0];

        (*CntrlPacket).Type[1] = CntrlString[counter];
        BCC2 ^= CntrlString[counter];
        counter++;

        (*CntrlPacket).Length[1] = CntrlString[counter];
        BCC2 ^= CntrlString[counter];
        counter++;        

        (*CntrlPacket).Filename = (unsigned char*) malloc (sizeof(char) * (*CntrlPacket).Length[1]);

        for (i = 0; i < (*CntrlPacket).Length[1]; i++)
        {
            (*CntrlPacket).Filename[i] = CntrlString[counter + i];
            BCC2 ^= CntrlString[counter + i];
        }
    }
    printf("A retornar de ReadCntrlPacket: %x\n", BCC2);
    return BCC2;
}

/* --- CreateConfirmMsg --- */

unsigned char *CreateConfirmMsg (unsigned short *ConfirmMsgSize, unsigned short CntrlField)
{
    unsigned char *ConfirmMsg = malloc (sizeof(char) * 5);

    *ConfirmMsgSize = 0;

    ConfirmMsg[(*ConfirmMsgSize)++] = (unsigned char) F;
    ConfirmMsg[(*ConfirmMsgSize)++] = (unsigned char) A;
    ConfirmMsg[(*ConfirmMsgSize)++] = (unsigned char) CntrlField;
    ConfirmMsg[(*ConfirmMsgSize)++] = (unsigned char) (A ^ CntrlField);
    ConfirmMsg[(*ConfirmMsgSize)++] = (unsigned char) F;

    return ConfirmMsg;
}

/* --- ReadConfirmMsg --- */

signed short ReadConfirmMsg (unsigned char *ConfirmMsg, unsigned short ConfirmMsgSize)
{
    unsigned short state = 0, CntrlField = 0 ;
    while (state < 5 && CntrlField != -1)
        switch (state)
        {
            case 0: // Verificar F na 1ª posição
                if (ConfirmMsg[state] == F)
                    state++;
                else
                    CntrlField = -1;

                break;

            case 1: // Verificar A na 2ª posição
                if (ConfirmMsg[state] == A)
                    state++;
                else
                    CntrlField = -1;

                break;

            case 2: // Verificar Campo de Controlo
                if (ConfirmMsg[state] == SET)
                {
                    state++;
                    CntrlField = SET;
                }
                else if (ConfirmMsg[state] == DISC)
                {
                    state++;
                    CntrlField = DISC;
                }
                else if (ConfirmMsg[state] == UA)
                {
                    state++;
                    CntrlField = UA;
                }
                else if (ConfirmMsg[state] == RR0)
                {
                    state++;
                    CntrlField = RR0;
                }
                else if (ConfirmMsg[state] == RR1)
                {
                    state++;
                    CntrlField = RR1;
                }
                else if (ConfirmMsg[state] == REJ0)
                {
                    state++;
                    CntrlField = REJ0;
                }
                else if (ConfirmMsg[state] == REJ1)
                {
                    state++;
                    CntrlField = REJ1;
                }
                else
                    CntrlField = -1;

                break;

            case 3: // Verificar BCC1
                if (ConfirmMsg[state] == ConfirmMsg[state - 1] ^ ConfirmMsg[state - 2])
                    state++;
                else
                    CntrlField = -1;

                break;

            case 4: // Verificar F na última posição
                if (ConfirmMsg[state] == F)
                    state++;
                else
                    CntrlField = -1;

                break;

            default:
                break;
        }

    return CntrlField;
}

/* --- CreateInfMsg --- */

unsigned char *CreateInfMsg (unsigned short *InfMsgSize, unsigned short CntrlField, unsigned char *DataField, unsigned short DataSize, unsigned short BCC2)
{
    int i, counter = 0;

    *InfMsgSize = 6 + DataSize;

    unsigned char *InfMsg = malloc (sizeof(char) * *InfMsgSize);

    InfMsg[counter++] = (unsigned char) F;
    InfMsg[counter++] = (unsigned char) A;
    InfMsg[counter++] = (unsigned char) CntrlField;
    InfMsg[counter++] = (unsigned char) (A ^ CntrlField);

    for (i = 0; i < DataSize; i++)
        InfMsg[counter + i] = DataField[i];
    counter += DataSize;

    InfMsg[counter++] = (unsigned char) BCC2;
    InfMsg[counter++] = (unsigned char) F;

    return InfMsg;
}

/* --- ReadInfMsg --- */

signed short ReadInfMsg (InfStruct *InfPacket, unsigned char *InfString, unsigned short InfMsgSize, signed short CntrlField)
{
    signed short Type, BCC2;
    unsigned short state = 0, counter = 0, StringSize, i;
    unsigned char *DataString, *CntrlString;
    DataStruct DataPacket;
    CntrlStruct CntrlPacket;

    /*for (i = 0; i < InfMsgSize; i++){
        printf("Mensagem a entrar[%d]: %x\n", i, InfString[i]);
    }*/
    //sleep(2);



   /*if (CntrlField == I0)
        CntrlField = I0;
    else if (CntrlField == RR1 || CntrlField == REJ1)
        CntrlField = I1;*/

    while (state < 7 && CntrlField != -1)
        switch (state)
        {
            case 0: // Verificar F
                //printf ("Comparar %x com %x.\n", InfString[counter], F);
                printf("Verificar F\n");
                if (InfString[counter] == F)
                {
                    //printf("Verificar F\n");
                    state++;
                    counter++;
                }
                else
                    return -1;

                break;

            case 1: // Verificar A
                //printf ("Comparar %x com %x.\n", InfString[counter], A);
                printf("Verificar A\n");
                if (InfString[counter] == A)
                {
                    state++;
                    counter++;
                }
                else
                    return -1;

                break;

            case 2: // Verificar C
                //printf ("Comparar %x com %x.\n", InfString[counter], CntrlField);
                printf("Counter: %d\n", counter);
                if (InfString[counter] == CntrlField)
                {
                    state++;
                    counter++;
                }
                else
                    return -1;

                break;

            case 3: // Verificar BCC1
                //printf ("Comparar %x com %x.\n", InfString[counter], InfString[counter - 1] ^ InfString[counter - 2]);
                printf("Verificar BCC1\n");
                if (InfString[counter] == InfString[counter - 1] ^ InfString[counter - 2])
                {
                    state++;
                    counter++;
                }
                else
                    return -1;

                break;

            case 4: // Verificar Dados
                printf("Verificar Dados\n");
                if (InfString[counter] == 1)
                {
                    StringSize = InfMsgSize - 6;
                    DataString = (unsigned char*) malloc (sizeof(unsigned char) * StringSize);
                    for (i = 0; i < StringSize; i++)
                        DataString[i] = InfString[counter + i];

                    BCC2 = ReadDataPacket (&DataPacket, DataString, StringSize);

                    /*

                        printf ("BCC2: %x\n", BCC2);

                        printf ("SeqNumber: %lu\n", DataPacket.SeqNumber);
                        printf ("DataSize: %lu\n", DataPacket.DataSize);
                        for (i = 0; i < DataPacket.DataSize; i++)
                            printf ("Data[%hu]: %x\n", i, DataPacket.Data[i]);

                    */

                    if (BCC2 == -1)
                        //Type = -1;
                        return -1;
                    else
                    {
                        Type = InfString[counter];
                        counter += StringSize;
                        state++;
                    }
                }
                else if (InfString[state] == 2 || InfString[state] == 3)
                {
                    StringSize = InfMsgSize - 6;
                    printf("StringSize1: %d\n",StringSize);
                    printf("InfMsgSize1: %d\n", InfMsgSize);
                    CntrlString = (unsigned char*) malloc (sizeof(unsigned char) * StringSize);
                    for (i = 0; i < StringSize; i++){
                        CntrlString[i] = InfString[counter + i];
                    }

                    BCC2 = ReadCntrlPacket (&CntrlPacket, CntrlString, StringSize);
                    printf("BCC2 new: %x\n", BCC2);
                    printf("StringSize2: %d\n",StringSize);
                    printf("InfMsgSize2: %d\n", InfMsgSize);

                    if (BCC2 == -1)
                        return -1;
                    else
                    {
                        Type = InfString[counter];
                        counter += StringSize;
                        printf("Counter apos: %d\n", counter);
                        state++;
                    }
                }
                else
                    //Type = -1;
                    return -1;

                break;

            case 5: // Verificar BCC2
                //printf ("Comparar %x com %x.\n", InfString[counter], BCC2);
                printf("Verificar BCC2 ...\n");
                printf ("Recebido: %x | Calculado: %x\n", InfString[counter], BCC2);
                if (InfString[counter] == BCC2)
                { 
                    printf("Bcc2 perfeito\n");
                    state++;
                    counter++;
                }
                else
                    //Type = -1;
                    return -1;

                break;

            case 6: // Verificar F
                //printf ("Comparar %x com %x.\n", InfString[counter], F);
                //printf("Verificar F...\n");
                if (InfString[counter] == F)
                {
                    printf("Encontrado segundo F\n");
                    state++;
                    counter++;
                }
                else
                    //Type = -1;
                    return -1;

                break;
        }

    if (Type == 1)
    {
        printf("Criacao InfPacket\n");
        (*InfPacket).InfType = Type;

        (*InfPacket).InfData.SeqNumber = DataPacket.SeqNumber;
        (*InfPacket).InfData.DataSize = DataPacket.DataSize;

        (*InfPacket).InfData.Data = (unsigned char*) malloc (sizeof(char) * (*InfPacket).InfData.DataSize);
        for (i = 0; i < (*InfPacket).InfData.DataSize; i++)
            (*InfPacket).InfData.Data[i] = DataPacket.Data[i];
    }
    else if (Type == 2 || Type == 3)
    {
        printf("Criacao InfPacket\n");
        printf("Type: %d\n",CntrlPacket.Type[0]);
        printf("Length: %d\n", CntrlPacket.Length[0]);
        printf("FileSize: ");
        for (i = 0; i < CntrlPacket.Length[0]; i++)
            printf ("%c", CntrlPacket.FileSize[i]);
        printf ("\n");
        

        printf ("Type #2: %u\n" , CntrlPacket.Type[1]);
        printf ("Length #2: %u\n", CntrlPacket.Length[1]);
        printf ("Filename: ");
        for (i = 0; i < CntrlPacket.Length[1]; i++)
            printf ("%c", CntrlPacket.Filename[i]);
            printf ("\n");

                        

        (*InfPacket).InfType = Type;
        printf("Infpacket %d\n", (*InfPacket).InfType );

        (*InfPacket).InfCntrl.Type[0] = CntrlPacket.Type[0];
        (*InfPacket).InfCntrl.Length[0] = CntrlPacket.Length[0];

        (*InfPacket).InfCntrl.FileSize = (unsigned char*) malloc (sizeof(char) * (*InfPacket).InfCntrl.Length[0]);
        for (i = 0; i < (*InfPacket).InfCntrl.Length[0]; i++)
            (*InfPacket).InfCntrl.FileSize[i] = CntrlPacket.FileSize[i];

        (*InfPacket).InfCntrl.Type[1] = CntrlPacket.Type[1];
        (*InfPacket).InfCntrl.Length[1] = CntrlPacket.Length[1];

        (*InfPacket).InfCntrl.Filename = (unsigned char*) malloc (sizeof(char) * (*InfPacket).InfCntrl.Length[1]);
        for (i = 0; i < (*InfPacket).InfCntrl.Length[1]; i++)
            (*InfPacket).InfCntrl.Filename[i] = CntrlPacket.Filename[i];
    }

    return Type;
}
