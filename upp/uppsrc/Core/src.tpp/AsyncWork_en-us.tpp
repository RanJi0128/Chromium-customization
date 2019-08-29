topic "AsyncWork and Async";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 AsyncWork and Async]]}}&]
[s3; &]
[s1;:noref: [@(0.0.255)3 template][3 _<][@(0.0.255)3 class][3 _][*@4;3 Ret][3 >]&]
[s1;:Upp`:`:AsyncWork`:`:class: [@(0.0.255) class]_[* AsyncWork]&]
[s2;%% Represents a job that can be executed in another thread and 
can return a value. It is similar to future/promise pattern, 
but it allows cancelation and is based on U`+`+ thread pool shared 
with CoWork. AsyncWork has pick constructor / operator.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:AsyncWork`:`:Do`(Upp`:`:Function`&`&`,Args`&`&`.`.`.args`): [@(0.0.255) tem
plate]_<_[@(0.0.255) class]_[*@4 Function], [@(0.0.255) class...]_[*@4 Args]>&]
[s5;:Upp`:`:AsyncWork`:`:Do`(Upp`:`:Function`&`&`,Args`&`&`.`.`.args`): [@(0.0.255) voi
d]_[* Do]([*@4 Function][@(0.0.255) `&`&]_[*@3 f], [*@4 Args][@(0.0.255) `&`&...]_args)&]
[s2;%% Schedules job [%-*@3 f] with parameters args to be asynchronously 
performed in (possibly) another thread.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:AsyncWork`:`:Cancel`(`): [@(0.0.255) void]_[* Cancel]()&]
[s2;%% Cancels the job.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:AsyncWork`:`:IsCanceled`(`): [@(0.0.255) static] [@(0.0.255) bool]_[* IsCancele
d]()&]
[s2;%% Returns true [*/ in the job routine] if the master AsyncWork 
was canceled.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:AsyncWork`:`:IsFinished`(`): [@(0.0.255) bool]_[* IsFinished]()&]
[s2;%% Returns true if job was finished.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:AsyncWork`:`:Get`(`): [*@4 Ret]_[* Get]()&]
[s2;%% Waits for job to be finished (if necessary), then returns 
the return value of [%-*@3 f]. If there was exception, it is rethrown.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:AsyncWork`:`:operator`~`(`): [*@4 Ret]_[* operator`~]()&]
[s2;%% Same as Get().&]
[s2;%% &]
[s3; &]
[s4; &]
[s5;:Upp`:`:AsyncWork`:`:`~AsyncWork`(`): [@(0.0.255) `~][* AsyncWork]()&]
[s2;%% If work has not be finished, destructor cancels it.&]
[s3; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Async]]}}&]
[s3; &]
[s5;:Upp`:`:Async`(Upp`:`:Function`&`&`,Args`&`&`.`.`.args`): [@(0.0.255) auto]_[* Async](
[_^Upp`:`:Function^ Function][@(0.0.255) `&`&]_[*@3 f], Args[@(0.0.255) `&`&...]_args)&]
[s2;%% Returns AsyncWork for given job [%-*@3 f] with [%- args].&]
[s0;@(0.0.255)3 ]]