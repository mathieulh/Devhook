       .set noreorder
        .set noat

        .global pspSdkSetK1
        .ent    pspSdkSetK1

pspSdkSetK1:
        move $2,$27
        jr	 $31
        move $27,$4

        .end pspSdkSetK1

        .global pspSdkGetK1
        .ent    pspSdkGetK1

pspSdkGetK1:
        jr   $31
        move $2,$27

        .end    pspSdkGetK1
