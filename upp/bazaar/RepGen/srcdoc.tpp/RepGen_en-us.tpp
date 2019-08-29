topic "RepGen - Short description and Tutorial";
[ $$0,0#00000000000000000000000000000000:Default]
[i448;a25;kKO9; $$1,0#37138531426314131252341829483380:structitem]
[l288;2 $$2,0#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[{_}%EN-US 
[ {{10000f0;t/25b/25@(113.42.0) [s0;%- [*@2;4 RepGen `- Very simple Report Generator (Short description)]]}}&]
[s0;>%- [*^topic`:`/`/RepGen`/srcdoc`/RepGen`$en`-us^1 `[en`]][*1  ][*^topic`:`/`/RepGen`/srcdoc`/RepGen`$ru`-ru^1 `[
ru`]]&]
[s1;%- [C class_][*C RepGen]&]
[s2; [3 RepGen `- Simple Report Generation Class]&]
[s0; &]
[s0;%- &]
[ {{10000F(128)G(128)@1 [s0;%- [* Short Description].]}}&]
[s0;2%- &]
[s0; [2 It work very simple:]&]
[s0; [*2 RepGen][2  write QTF report by QTF template.]&]
[s0; [2 If RepGen found ##`-variables in template it replace by real 
values.]&]
[s0;2 &]
[s0; [2 Work of ][*2 RepGen ][2 controlled by Some ][*2 CallBacks][2 , wich 
is individual for each report type. ]&]
[s0;2 &]
[s0; [2 For each type of reports can be ][*/2 several various][2  templates, 
that your users can select before executing the report.]&]
[s3;2%- &]
[ {{10000F(128)G(128)@1 [s0;%- [* Tutorial]]}}&]
[s0;2%- &]
[s0; Now in more detail.&]
[s0; &]
[s0; [* RepGen] can generate a report based on qtf`-template. In other 
words, for the creation of the final report like this type:&]
[s0; &]
[ {{10000@1 [s0;= AddressBookXML2`+RepGen&]
[s0;= Test report&]
[s0;= [2 (Use UWord for Edit)]&]
[s0;= &]
[s0;= `"Simple table`"&]
[ {{2202:1959:2342:3497h1;b4/15 [s0;= [*+117 Name]]
:: [s0;= [*+117 Surname]]
:: [s0;= [*+117 Address]]
:: [s0;= [*+117 E`-mail]]
::b0/15 [s0;=  Petr]
:: [s0;= Petrov]
:: [s0;= Moscow]
:: [s0;= petr`@petrovich.ru ]
:: [s0;=  Ivan]
:: [s0;= Ivanov]
:: [s0;= Ekaterinburg]
:: [s0;= ivan`@ivanovich.ru ]
:: [s0;=  Sidor]
:: [s0;= Sidorov]
:: [s0;= Kazan]
:: [s0;= sidor`@sidorov.ru ]
::t4/15-3 [s0;> TOTAL [*/ 3] ADDRESSES]
::t0/15-2 [s0;%- ]
::-1 [s0;%- ]
:: [s0;%- ]}}&]
[s0;= &]
[s0; ]}}&]
[s0; &]
[s0; The [* template] for this report should be like this:&]
[s0; &]
[ {{10000@1 [s0;= AddressBookXML2`+RepGen&]
[s0;= Test report&]
[s0;= [2 (Use UWord for Edit)]&]
[s0;= &]
[s0;= `"Simple table`"&]
[ {{2500:2500:2500:2500h1;b4/15 [s0;= [*+117 Name]]
:: [s0;= [*+117 Surname]]
:: [s0;= [*+117 Address]]
:: [s0;= [*+117 E`-mail]]
::b0/15 [s0;= ##BT ##NAME]
:: [s0;= ##SURNAME]
:: [s0;= ##ADDRESS]
:: [s0;= ##EMAIL ##ET]
::t4/15-3 [s0;> TOTAL [*/ ##TOTAL] ADDRESSES]
::t0/15-2 [s0;%- ]
::-1 [s0;%- ]
:: [s0;%- ]}}&]
[s0;= &]
[s0; ]}}&]
[s0; &]
[s0; To deal with this template to create an object of type [* RepGen].&]
[s0; &]
[s0; Then you need to [* create several callbacks], which will set 
the logic of this report and actually implement the substitution 
of real data.&]
[s0; &]
[s0; Additionally you must provide an [* GUI interface choose the right 
template]. But that is another story, as a result of choice in 
RepGen object should get the content qtf`-template.&]
[s0; &]
[s0; Reference [^topic`:`/`/RepGen`/src`/RepGen`$en`-us`#RepGen`:`:class^ here]&]
[s0; &]
[s0; ChangeLog [^topic`:`/`/RepGen`/srcdoc`/changelog`$en`-us^ here]&]
[s0; &]
[s0; This article in [^topic`:`/`/RepGen`/srcdoc`/RepGen`$ru`-ru^ Russian]]