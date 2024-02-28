; ModuleID = 'main.bc'
source_filename = "main.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$main = comdat nodeduplicate

$sancov.module_ctor_trace_pc_guard = comdat any

@.str = internal constant { [5 x i8], [27 x i8] } { [5 x i8] c"yeah\00", [27 x i8] zeroinitializer }, align 32, !dbg !0
@.str.1 = internal constant { [2 x i8], [30 x i8] } { [2 x i8] c"a\00", [30 x i8] zeroinitializer }, align 32, !dbg !7
@.str.2 = internal constant { [5 x i8], [27 x i8] } { [5 x i8] c"what\00", [27 x i8] zeroinitializer }, align 32, !dbg !12
@.str.3 = internal constant { [7 x i8], [25 x i8] } { [7 x i8] c"whatup\00", [25 x i8] zeroinitializer }, align 32, !dbg !14
@__afl_area_ptr = external global ptr
@__sancov_gen_ = private global [6 x i32] zeroinitializer, section "__sancov_guards", comdat($main), align 4
@__start___sancov_guards = extern_weak hidden global ptr
@__stop___sancov_guards = extern_weak hidden global ptr
@__lscov_area_ptr = external global ptr
@__lscov_prev_loc = external thread_local global i32
@___asan_gen_ = private constant [7 x i8] c"main.c\00", align 1
@___asan_gen_.4 = private unnamed_addr constant [5 x i8] c".str\00", align 1
@___asan_gen_.5 = private unnamed_addr constant [7 x i8] c".str.1\00", align 1
@___asan_gen_.6 = private unnamed_addr constant [7 x i8] c".str.2\00", align 1
@___asan_gen_.7 = private unnamed_addr constant [7 x i8] c".str.3\00", align 1
@llvm.compiler.used = appending global [5 x ptr] [ptr @__sancov_gen_, ptr @.str, ptr @.str.1, ptr @.str.2, ptr @.str.3], section "llvm.metadata"
@0 = internal global [4 x { i64, i64, i64, i64, i64, i64, i64, i64 }] [{ i64, i64, i64, i64, i64, i64, i64, i64 } { i64 ptrtoint (ptr @.str to i64), i64 5, i64 32, i64 ptrtoint (ptr @___asan_gen_.4 to i64), i64 ptrtoint (ptr @___asan_gen_ to i64), i64 0, i64 0, i64 -1 }, { i64, i64, i64, i64, i64, i64, i64, i64 } { i64 ptrtoint (ptr @.str.1 to i64), i64 2, i64 32, i64 ptrtoint (ptr @___asan_gen_.5 to i64), i64 ptrtoint (ptr @___asan_gen_ to i64), i64 0, i64 0, i64 -1 }, { i64, i64, i64, i64, i64, i64, i64, i64 } { i64 ptrtoint (ptr @.str.2 to i64), i64 5, i64 32, i64 ptrtoint (ptr @___asan_gen_.6 to i64), i64 ptrtoint (ptr @___asan_gen_ to i64), i64 0, i64 0, i64 -1 }, { i64, i64, i64, i64, i64, i64, i64, i64 } { i64 ptrtoint (ptr @.str.3 to i64), i64 7, i64 32, i64 ptrtoint (ptr @___asan_gen_.7 to i64), i64 ptrtoint (ptr @___asan_gen_ to i64), i64 0, i64 0, i64 -1 }]
@llvm.used = appending global [3 x ptr] [ptr @sancov.module_ctor_trace_pc_guard, ptr @asan.module_ctor, ptr @asan.module_dtor], section "llvm.metadata"
@llvm.global_ctors = appending global [2 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 2, ptr @sancov.module_ctor_trace_pc_guard, ptr @sancov.module_ctor_trace_pc_guard }, { i32, ptr, ptr } { i32 1, ptr @asan.module_ctor, ptr null }]
@llvm.global_dtors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 1, ptr @asan.module_dtor, ptr null }]

; Function Attrs: nofree nounwind sanitize_address uwtable
define i32 @main(i32 noundef %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #0 comdat !dbg !27 {
entry:
  %0 = load i32, ptr @__lscov_prev_loc, align 4, !dbg !36, !nosanitize !37
  %1 = load ptr, ptr @__lscov_area_ptr, align 8, !dbg !36, !nosanitize !37
  %2 = xor i32 %0, 43618, !dbg !36
  %3 = getelementptr ptr, ptr %1, i32 %2, !dbg !36
  %4 = load i8, ptr %3, align 1, !dbg !36, !nosanitize !37
  %5 = add i8 %4, 1, !dbg !36
  store i8 %5, ptr %3, align 1, !dbg !36, !nosanitize !37
  store i32 21809, ptr @__lscov_prev_loc, align 4, !dbg !36, !nosanitize !37
  %6 = load i32, ptr @__sancov_gen_, align 4, !dbg !36, !nosanitize !37
  %7 = load ptr, ptr @__afl_area_ptr, align 8, !dbg !36, !nosanitize !37
  %8 = getelementptr i8, ptr %7, i32 %6, !dbg !36
  %9 = load i8, ptr %8, align 1, !dbg !36, !nosanitize !37
  %10 = add i8 %9, 1, !dbg !36
  %11 = icmp eq i8 %10, 0, !dbg !36
  %12 = zext i1 %11 to i8, !dbg !36
  %13 = add i8 %10, %12, !dbg !36
  store i8 %13, ptr %8, align 1, !dbg !36, !nosanitize !37
  call void @llvm.dbg.value(metadata i32 %argc, metadata !34, metadata !DIExpression()), !dbg !38
  call void @llvm.dbg.value(metadata ptr %argv, metadata !35, metadata !DIExpression()), !dbg !38
  %cmp = icmp eq i32 %argc, 1, !dbg !39
  br i1 %cmp, label %if.then, label %entry.if.end5_crit_edge, !dbg !41

entry.if.end5_crit_edge:                          ; preds = %entry
  %14 = load i32, ptr @__lscov_prev_loc, align 4, !dbg !41, !nosanitize !37
  %15 = load ptr, ptr @__lscov_area_ptr, align 8, !dbg !41, !nosanitize !37
  %16 = xor i32 %14, 55425, !dbg !41
  %17 = getelementptr ptr, ptr %15, i32 %16, !dbg !41
  %18 = load i8, ptr %17, align 1, !dbg !41, !nosanitize !37
  %19 = add i8 %18, 1, !dbg !41
  store i8 %19, ptr %17, align 1, !dbg !41, !nosanitize !37
  store i32 27712, ptr @__lscov_prev_loc, align 4, !dbg !41, !nosanitize !37
  %20 = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__sancov_gen_ to i64), i64 4) to ptr), align 4, !dbg !41, !nosanitize !37
  %21 = load ptr, ptr @__afl_area_ptr, align 8, !dbg !41, !nosanitize !37
  %22 = getelementptr i8, ptr %21, i32 %20, !dbg !41
  %23 = load i8, ptr %22, align 1, !dbg !41, !nosanitize !37
  %24 = add i8 %23, 1, !dbg !41
  %25 = icmp eq i8 %24, 0, !dbg !41
  %26 = zext i1 %25 to i8, !dbg !41
  %27 = add i8 %24, %26, !dbg !41
  store i8 %27, ptr %22, align 1, !dbg !41, !nosanitize !37
  br label %if.end5, !dbg !41

if.then:                                          ; preds = %entry
  %28 = load i32, ptr @__lscov_prev_loc, align 4, !dbg !42, !nosanitize !37
  %29 = load ptr, ptr @__lscov_area_ptr, align 8, !dbg !42, !nosanitize !37
  %30 = xor i32 %28, 12109, !dbg !42
  %31 = getelementptr ptr, ptr %29, i32 %30, !dbg !42
  %32 = load i8, ptr %31, align 1, !dbg !42, !nosanitize !37
  %33 = add i8 %32, 1, !dbg !42
  store i8 %33, ptr %31, align 1, !dbg !42, !nosanitize !37
  store i32 6054, ptr @__lscov_prev_loc, align 4, !dbg !42, !nosanitize !37
  %34 = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__sancov_gen_ to i64), i64 8) to ptr), align 4, !dbg !42, !nosanitize !37
  %35 = load ptr, ptr @__afl_area_ptr, align 8, !dbg !42, !nosanitize !37
  %36 = getelementptr i8, ptr %35, i32 %34, !dbg !42
  %37 = load i8, ptr %36, align 1, !dbg !42, !nosanitize !37
  %38 = add i8 %37, 1, !dbg !42
  %39 = icmp eq i8 %38, 0, !dbg !42
  %40 = zext i1 %39 to i8, !dbg !42
  %41 = add i8 %38, %40, !dbg !42
  store i8 %41, ptr %36, align 1, !dbg !42, !nosanitize !37
  %call = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str), !dbg !42
  %42 = ptrtoint ptr %argv to i64, !dbg !44
  %43 = lshr i64 %42, 3, !dbg !44
  %44 = add i64 %43, 2147450880, !dbg !44
  %45 = inttoptr i64 %44 to ptr, !dbg !44
  %46 = load i8, ptr %45, align 1, !dbg !44
  %47 = icmp ne i8 %46, 0, !dbg !44
  br i1 %47, label %48, label %49, !dbg !44

48:                                               ; preds = %if.then
  call void @__asan_report_load8(i64 %42) #4, !dbg !44
  unreachable, !dbg !44

49:                                               ; preds = %if.then
  %50 = load ptr, ptr %argv, align 8, !dbg !44, !tbaa !46
  %cmp1 = icmp eq ptr %50, @.str.1, !dbg !50
  %.str.2..str.3 = select i1 %cmp1, ptr @.str.2, ptr @.str.3
  %51 = select i1 %cmp1, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__sancov_gen_ to i64), i64 12) to ptr), ptr inttoptr (i64 add (i64 ptrtoint (ptr @__sancov_gen_ to i64), i64 16) to ptr), !dbg !51
  %52 = load ptr, ptr @__afl_area_ptr, align 8, !dbg !51, !nosanitize !37
  %53 = load i32, ptr %51, align 4, !dbg !51, !nosanitize !37
  %54 = getelementptr i8, ptr %52, i32 %53, !dbg !51
  %55 = load i8, ptr %54, align 1, !dbg !51, !nosanitize !37
  %56 = add i8 %55, 1, !dbg !51
  %57 = icmp eq i8 %56, 0, !dbg !51
  %58 = zext i1 %57 to i8, !dbg !51
  %59 = add i8 %56, %58, !dbg !51
  store i8 %59, ptr %54, align 1, !dbg !51, !nosanitize !37
  %call3 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull %.str.2..str.3), !dbg !51
  br label %if.end5, !dbg !52

if.end5:                                          ; preds = %entry.if.end5_crit_edge, %49
  %60 = load i32, ptr @__lscov_prev_loc, align 4, !dbg !52, !nosanitize !37
  %61 = load ptr, ptr @__lscov_area_ptr, align 8, !dbg !52, !nosanitize !37
  %62 = xor i32 %60, 62065, !dbg !52
  %63 = getelementptr ptr, ptr %61, i32 %62, !dbg !52
  %64 = load i8, ptr %63, align 1, !dbg !52, !nosanitize !37
  %65 = add i8 %64, 1, !dbg !52
  store i8 %65, ptr %63, align 1, !dbg !52, !nosanitize !37
  store i32 31032, ptr @__lscov_prev_loc, align 4, !dbg !52, !nosanitize !37
  ret i32 0, !dbg !52
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

declare void @__sanitizer_cov_trace_pc_guard_init(ptr, ptr)

; Function Attrs: nounwind uwtable
define internal void @sancov.module_ctor_trace_pc_guard() #3 comdat {
  %1 = load i32, ptr @__lscov_prev_loc, align 4, !nosanitize !37
  %2 = load ptr, ptr @__lscov_area_ptr, align 8, !nosanitize !37
  %3 = xor i32 %1, 53987
  %4 = getelementptr ptr, ptr %2, i32 %3
  %5 = load i8, ptr %4, align 1, !nosanitize !37
  %6 = add i8 %5, 1
  store i8 %6, ptr %4, align 1, !nosanitize !37
  store i32 26993, ptr @__lscov_prev_loc, align 4, !nosanitize !37
  call void @__sanitizer_cov_trace_pc_guard_init(ptr @__start___sancov_guards, ptr @__stop___sancov_guards)
  ret void
}

declare void @__asan_report_load8(i64)

declare void @__asan_register_globals(i64, i64)

declare void @__asan_unregister_globals(i64, i64)

declare void @__asan_init()

; Function Attrs: nounwind uwtable
define internal void @asan.module_ctor() #3 {
  call void @__asan_init()
  call void @__asan_version_mismatch_check_v8()
  call void @__asan_register_globals(i64 ptrtoint (ptr @0 to i64), i64 4)
  ret void
}

declare void @__asan_version_mismatch_check_v8()

; Function Attrs: nounwind uwtable
define internal void @asan.module_dtor() #3 {
  call void @__asan_unregister_globals(i64 ptrtoint (ptr @0 to i64), i64 4)
  ret void
}

attributes #0 = { nofree nounwind sanitize_address uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nounwind uwtable }
attributes #4 = { nomerge }

!llvm.dbg.cu = !{!19}
!llvm.module.flags = !{!21, !22, !23, !24, !25}
!llvm.ident = !{!26}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 5, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "main.c", directory: "/home/gwangmu/Projects/LogicStateCoverage/testbed/small_proj", checksumkind: CSK_MD5, checksum: "53a28f2dd16e79b91723ba1a15ff6ec7")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 40, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 5)
!7 = !DIGlobalVariableExpression(var: !8, expr: !DIExpression())
!8 = distinct !DIGlobalVariable(scope: null, file: !2, line: 6, type: !9, isLocal: true, isDefinition: true)
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 16, elements: !10)
!10 = !{!11}
!11 = !DISubrange(count: 2)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(scope: null, file: !2, line: 7, type: !3, isLocal: true, isDefinition: true)
!14 = !DIGlobalVariableExpression(var: !15, expr: !DIExpression())
!15 = distinct !DIGlobalVariable(scope: null, file: !2, line: 9, type: !16, isLocal: true, isDefinition: true)
!16 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 56, elements: !17)
!17 = !{!18}
!18 = !DISubrange(count: 7)
!19 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2, producer: "clang version 15.0.5", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, globals: !20, splitDebugInlining: false, nameTableKind: None)
!20 = !{!0, !7, !12, !14}
!21 = !{i32 7, !"Dwarf Version", i32 5}
!22 = !{i32 2, !"Debug Info Version", i32 3}
!23 = !{i32 1, !"wchar_size", i32 4}
!24 = !{i32 7, !"PIC Level", i32 2}
!25 = !{i32 7, !"uwtable", i32 2}
!26 = !{!"clang version 15.0.5"}
!27 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 3, type: !28, scopeLine: 3, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !19, retainedNodes: !33)
!28 = !DISubroutineType(types: !29)
!29 = !{!30, !30, !31}
!30 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !32, size: 64)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!33 = !{!34, !35}
!34 = !DILocalVariable(name: "argc", arg: 1, scope: !27, file: !2, line: 3, type: !30)
!35 = !DILocalVariable(name: "argv", arg: 2, scope: !27, file: !2, line: 3, type: !31)
!36 = !DILocation(line: 3, scope: !27)
!37 = !{}
!38 = !DILocation(line: 0, scope: !27)
!39 = !DILocation(line: 4, column: 12, scope: !40)
!40 = distinct !DILexicalBlock(scope: !27, file: !2, line: 4, column: 7)
!41 = !DILocation(line: 4, column: 7, scope: !27)
!42 = !DILocation(line: 5, column: 5, scope: !43)
!43 = distinct !DILexicalBlock(scope: !40, file: !2, line: 4, column: 18)
!44 = !DILocation(line: 6, column: 9, scope: !45)
!45 = distinct !DILexicalBlock(scope: !43, file: !2, line: 6, column: 9)
!46 = !{!47, !47, i64 0}
!47 = !{!"any pointer", !48, i64 0}
!48 = !{!"omnipotent char", !49, i64 0}
!49 = !{!"Simple C/C++ TBAA"}
!50 = !DILocation(line: 6, column: 17, scope: !45)
!51 = !DILocation(line: 0, scope: !45)
!52 = !DILocation(line: 11, column: 3, scope: !27)
