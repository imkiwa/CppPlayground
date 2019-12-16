; ModuleID = 'vptr.cpp'
source_filename = "vptr.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-darwin19.0.0"

%class.Derived = type { %class.Base }
%class.Base = type { i32 (...)** }

@_ZTV7Derived = linkonce_odr unnamed_addr constant { [8 x i8*] } { [8 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7DerivedD1Ev to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7DerivedD0Ev to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7Derived12pureVirtual1Ev to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7Derived12pureVirtual2Ev to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7Derived12pureVirtual3Ev to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7Derived12pureVirtual4Ev to i8*)] }, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
@_ZTS7Derived = linkonce_odr constant [9 x i8] c"7Derived\00", align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS4Base = linkonce_odr constant [6 x i8] c"4Base\00", align 1
@_ZTI4Base = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, align 8
@_ZTI7Derived = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, align 8
@_ZTV4Base = linkonce_odr unnamed_addr constant { [8 x i8*] } { [8 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*), i8* bitcast (void (%class.Base*)* @_ZN4BaseD1Ev to i8*), i8* bitcast (void (%class.Base*)* @_ZN4BaseD0Ev to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*), i8* bitcast (void ()* @__cxa_pure_virtual to i8*)] }, align 8

; Function Attrs: noinline norecurse nounwind optnone
define i32 @main() #0 {
entry:
  %d = alloca %class.Derived, align 8
  call void @_ZN7DerivedC1Ev(%class.Derived* %d) #4
  call void @_ZN7Derived12pureVirtual1Ev(%class.Derived* %d)
  call void @_ZN7DerivedD1Ev(%class.Derived* %d) #4
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7DerivedC1Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  call void @_ZN7DerivedC2Ev(%class.Derived* %this1) #4
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7Derived12pureVirtual1Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7DerivedD1Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  call void @_ZN7DerivedD2Ev(%class.Derived* %this1) #4
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7DerivedC2Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  %0 = bitcast %class.Derived* %this1 to %class.Base*
  call void @_ZN4BaseC2Ev(%class.Base* %0) #4
  %1 = bitcast %class.Derived* %this1 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* @_ZTV7Derived, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %1, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN4BaseC2Ev(%class.Base* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Base*, align 8
  store %class.Base* %this, %class.Base** %this.addr, align 8
  %this1 = load %class.Base*, %class.Base** %this.addr, align 8
  %0 = bitcast %class.Base* %this1 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* @_ZTV4Base, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %0, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7DerivedD0Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  call void @_ZN7DerivedD1Ev(%class.Derived* %this1) #4
  %0 = bitcast %class.Derived* %this1 to i8*
  call void @_ZdlPv(i8* %0) #5
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7Derived12pureVirtual2Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7Derived12pureVirtual3Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7Derived12pureVirtual4Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN4BaseD1Ev(%class.Base* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Base*, align 8
  store %class.Base* %this, %class.Base** %this.addr, align 8
  %this1 = load %class.Base*, %class.Base** %this.addr, align 8
  call void @llvm.trap() #6
  unreachable

return:                                           ; No predecessors!
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN4BaseD0Ev(%class.Base* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Base*, align 8
  store %class.Base* %this, %class.Base** %this.addr, align 8
  %this1 = load %class.Base*, %class.Base** %this.addr, align 8
  call void @llvm.trap() #6
  unreachable

return:                                           ; No predecessors!
  ret void
}

declare void @__cxa_pure_virtual() unnamed_addr

; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #2

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(i8*) #3

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN7DerivedD2Ev(%class.Derived* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Derived*, align 8
  store %class.Derived* %this, %class.Derived** %this.addr, align 8
  %this1 = load %class.Derived*, %class.Derived** %this.addr, align 8
  %0 = bitcast %class.Derived* %this1 to %class.Base*
  call void @_ZN4BaseD2Ev(%class.Base* %0) #4
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr void @_ZN4BaseD2Ev(%class.Base* %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %class.Base*, align 8
  store %class.Base* %this, %class.Base** %this.addr, align 8
  %this1 = load %class.Base*, %class.Base** %this.addr, align 8
  ret void
}

attributes #0 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { cold noreturn nounwind }
attributes #3 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { builtin nounwind }
attributes #6 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Apple clang version 11.0.0 (clang-1100.0.33.8)"}
