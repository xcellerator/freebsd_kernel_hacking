; if (ip->i_flag & IN_ACCESS) {
ffffffff80ef7550:	41 8b 4e 48          	mov    0x48(%r14),%ecx
ffffffff80ef7554:	f6 c1 01             	test   $0x1,%cl
ffffffff80ef7557:	74 3b                	je     ffffffff80ef7594 <ufs_itimes_locked+0xd4>
ffffffff80ef7559:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
ffffffff80ef755d:	f7 c1 00 04 00 00    	test   $0x400,%ecx
; DIP_SET(ip, i_atime, ts.tv_sec);
ffffffff80ef7563:	75 09                	jne    ffffffff80ef756e <ufs_itimes_locked+0xae>
ffffffff80ef7565:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef7569:	89 41 10             	mov    %eax,0x10(%rcx)
ffffffff80ef756c:	eb 08                	jmp    ffffffff80ef7576 <ufs_itimes_locked+0xb6>
ffffffff80ef756e:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef7572:	48 89 41 20          	mov    %rax,0x20(%rcx)
ffffffff80ef7576:	41 f6 46 49 04       	testb  $0x4,0x49(%r14)
ffffffff80ef757b:	8b 45 e8             	mov    -0x18(%rbp),%eax
; DIP_SET(ip, i_atimensec, ts.tv_nsec);
ffffffff80ef757e:	75 0a                	jne    ffffffff80ef758a <ufs_itimes_locked+0xca>
ffffffff80ef7580:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef7584:	48 83 c1 14          	add    $0x14,%rcx
ffffffff80ef7588:	eb 08                	jmp    ffffffff80ef7592 <ufs_itimes_locked+0xd2>
ffffffff80ef758a:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef758e:	48 83 c1 44          	add    $0x44,%rcx
ffffffff80ef7592:	89 01                	mov    %eax,(%rcx)
; }

; if (ip->ip_flag & IN_UPDATE) {
ffffffff80ef7594:	41 8b 4e 48          	mov    0x48(%r14),%ecx
ffffffff80ef7598:	f6 c1 04             	test   $0x4,%cl
ffffffff80ef759b:	74 37                	je     ffffffff80ef75d4 <ufs_itimes_locked+0x114>
ffffffff80ef759d:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
ffffffff80ef75a1:	f7 c1 00 04 00 00    	test   $0x400,%ecx
; DIP_SET(ip, i_mtime, ts.tv_sec);
ffffffff80ef75a7:	75 09                	jne    ffffffff80ef75b2 <ufs_itimes_locked+0xf2>
ffffffff80ef75a9:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef75ad:	89 41 18             	mov    %eax,0x18(%rcx)
ffffffff80ef75b0:	eb 08                	jmp    ffffffff80ef75ba <ufs_itimes_locked+0xfa>
ffffffff80ef75b2:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef75b6:	48 89 41 28          	mov    %rax,0x28(%rcx)
ffffffff80ef75ba:	41 f6 46 49 04       	testb  $0x4,0x49(%r14)
ffffffff80ef75bf:	8b 45 e8             	mov    -0x18(%rbp),%eax
; DIP_SET(ip, i_mtimensec, ts.tv_nsec);
ffffffff80ef75c2:	75 09                	jne    ffffffff80ef75cd <ufs_itimes_locked+0x10d>
ffffffff80ef75c4:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef75c8:	89 41 1c             	mov    %eax,0x1c(%rcx)
ffffffff80ef75cb:	eb 07                	jmp    ffffffff80ef75d4 <ufs_itimes_locked+0x114>
ffffffff80ef75cd:	49 8b 4e 38          	mov    0x38(%r14),%rcx
ffffffff80ef75d1:	89 41 40             	mov    %eax,0x40(%rcx)
; }

; if (ip->i_flag & IN_CHANGE) {
ffffffff80ef75d4:	41 8b 4e 48          	mov    0x48(%r14),%ecx
ffffffff80ef75d8:	f6 c1 02             	test   $0x2,%cl
ffffffff80ef75db:	0f 84 02 ff ff ff    	je     ffffffff80ef74e3 <ufs_itimes_locked+0x23>
ffffffff80ef75e1:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
ffffffff80ef75e5:	f7 c1 00 04 00 00    	test   $0x400,%ecx
; DIP_SET(ip, i_ctime, ts.tv_sec);
ffffffff80ef75eb:	75 09                	jne    ffffffff80ef75f6 <ufs_itimes_locked+0x136>
* ffffffff80ef75ed:	49 8b 4e 38          	mov    0x38(%r14),%rcx
* ffffffff80ef75f1:	89 41 20             	mov    %eax,0x20(%rcx)
ffffffff80ef75f4:	eb 08                	jmp    ffffffff80ef75fe <ufs_itimes_locked+0x13e>
* ffffffff80ef75f6:	49 8b 4e 38          	mov    0x38(%r14),%rcx
* ffffffff80ef75fa:	48 89 41 30          	mov    %rax,0x30(%rcx)
ffffffff80ef75fe:	41 f6 46 49 04       	testb  $0x4,0x49(%r14)
ffffffff80ef7603:	8b 45 e8             	mov    -0x18(%rbp),%eax
; DIP_SET(ip, i_ctimensec, ts.tv_nsec);
; DIP_SET(ip, i_modrev, DIP(ip, i_modrev) + 1);
ffffffff80ef7606:	75 1f                	jne    ffffffff80ef7627 <ufs_itimes_locked+0x167>
* ffffffff80ef7608:	49 8b 4e 38          	mov    0x38(%r14),%rcx
* ffffffff80ef760c:	89 41 24             	mov    %eax,0x24(%rcx)
ffffffff80ef760f:	41 f6 46 49 04       	testb  $0x4,0x49(%r14)
ffffffff80ef7614:	74 1f                	je     ffffffff80ef7635 <ufs_itimes_locked+0x175>
ffffffff80ef7616:	49 8b 46 38          	mov    0x38(%r14),%rax
ffffffff80ef761a:	48 83 80 e8 00 00 00 	addq   $0x1,0xe8(%rax)
ffffffff80ef7621:	01 
ffffffff80ef7622:	e9 bc fe ff ff       	jmpq   ffffffff80ef74e3 <ufs_itimes_locked+0x23>
* ffffffff80ef7627:	49 8b 4e 38          	mov    0x38(%r14),%rcx
* ffffffff80ef762b:	89 41 48             	mov    %eax,0x48(%rcx)
ffffffff80ef762e:	41 f6 46 49 04       	testb  $0x4,0x49(%r14)
ffffffff80ef7633:	75 e1                	jne    ffffffff80ef7616 <ufs_itimes_locked+0x156>
* ffffffff80ef7635:	49 8b 46 38          	mov    0x38(%r14),%rax
ffffffff80ef7639:	48 83 40 78 01       	addq   $0x1,0x78(%rax)
ffffffff80ef763e:	e9 a0 fe ff ff       	jmpq   ffffffff80ef74e3 <ufs_itimes_locked+0x23>
; }
