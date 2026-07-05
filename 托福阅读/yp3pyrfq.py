import random
import re

# Word文档的内容（已从上传文件中提取，并进行了初步处理以方便解析）
# 实际程序运行中，这些数据应通过读取文件获得，但在此环境中我们直接使用提取出的文本。
WORD_DATA = """
illusive,adj.,迷惑人的；错觉的
illustrate,vt.,（用图或例子）说明，阐明
illustration,n.,例证；插图
image,n.,图像，肖像；形象
imaginative,adj.,富于想象力的
imaginary,adj.,想象中的，虚构的
imagery,n.,（艺术、文学）肖像，形象；形象化的比喻
imbibe,v.,吸；吸收
imitate,vt.,仿效，模仿
imitation,n.,模仿；仿制品
immediate,adj.,立即的；（在时空上、关系上）最接近的；目前的
immediately,adv.,立即，马上
immense,adj.,巨大的；无边的
immigrant,n.,（自外国移入）移民
immigration,n.,外来的移民, 移居入境
immobile,adj.,固定的，不动的
immobilize,vt.,使不动，使固定
demobilize,vt.,复员
immune,adj.,（对...）免疫的，不受影响的
immune system,词组,免疫系统
immunity,n.,免疫，免疫性
impact,n.,影响
impact,n.,撞击，冲击
impair,vt.,损害，使弱
impart,vt.,传授；传递，传达
impartial,adj.,公正的，无偏见的
impede,vt.,妨碍，阻碍
impediment,n.,妨碍，阻碍（物）
impending,adj.,即将发生的，逼近的
imperative,adj.,必要的；强制的
imperative,n.,命令；需要，必要的事
impermeable,adj.,不可渗透的；不透水的
impersonal,adj.,不受个人感情支配的，客观的，无私的
impersonate,vt.,模仿，扮演
impersonation,n.,模仿，扮演
impetus,n.,推动力；激励物
implement,n.,工具，器具
implement,vt.,实行，实施
implication,n.,暗示，意义
implication,n.,牵连
implicit,adj.,不明确的，含蓄的
imply,vt.,暗示，意味
import,vt./n.,（贸易）进口
impose,vt.,把...强加于；施加影响
imposing,adj.,使人难忘的, 壮丽的
impressive,adj.,给人深刻印象的；威严的
imprint,n.,印痕，痕迹；深刻的印象
imprint,vt.,压印，牢记
imprisonment,n.,监禁
improbable,adj.,不可能的
improvise,vt.,即席创作，即兴表演
improvision,n.,即席创作
improviser,n.,即席演奏者
impulse,n.,冲动；刺激，驱使
in conjunction with,词组,结合
in accord with,词组,与...一致
in common,词组,共有，有共同之处
in scale,词组,相称，成比例
in terms of,词组,根据；就...而论
inaccessible,adj.,难达到的,不可及的;不能得到的
inactivate,vt.,使...不活跃
inadequate,adj.,不充分的，不适当的
inanimate,adj.,无生命的
inappropriate,adj.,不适当的，不相称的
appropriate,adj.,适当的
appropriate,vt.,拨款；占用
inaugurate,v.,创新，开辟
inaugurate,v.,使就职
incapacitate,vt.,使失去能力；使不胜任
incense,n.,熏香；焚香的烟
incense,vt.,激怒
incentive,n.,动机
incessant,adj.,不停的，连续的
incident,n.,事件
incidentally,adv.,附带地，偶然地
incidence,n.,发生，出现；发生频率
coincident,adj.,一致的，符合的；巧合的
coincidence,n.,一致；同时发生或同时存在的事
incisive,adj.,深刻的，尖锐的
incise,vt.,切割
inclination,n.,嗜好，倾向
inclined,adj.,倾向...的
inclined,adj.,倾斜的，有坡度的
inclusive,adj.,包含的，包括的
incompatible,adj.,合不来的,相互不协调的;不兼容的
incomprehensible,adj.,不能理解的，费解的
incongruity,n.,不一致，不和谐
incongruous,adj.,不协调的，不一致的
incorpoarate,vt.,合并，并入；包含
incorporation,n.,结合，合并
incredible,adj.,难以置信的，惊人的
incredulity,n.,怀疑，不相信
incur,vt.,招致，引起
indefinite,adj.,模糊的；不明确的
indefinitely,adv.,不确定地；无穷尽地
indent,vt.,切割成锯齿状
indented,adj.,锯齿状的，高低不平的
indenture,vt.,以契约约束
indicate,vt.,指示，指出；象征，表明
indicative,adj.,指示的，预示的
indifferent,adj.,不感兴趣的，漠不关心的
indifference,n.,冷漠，不关心
indigestion,n.,消化不良
indiscriminately,adv.,随意地；不加区别地
indiscrimination,n.,歧视
indispensable,adj.,不可缺少的，必需的
individualism,n.,个人主义
induce,vt.,诱使，促使；导致
inducible,adj.,可诱导的
industrial,adj.,工业的
industrialization,n.,工业化
industrialized,vt.,工业化的
industrious,adj.,勤勉的
ineligible,adj.,无资格的；不适当的
inert,adj.,惰性的；不活泼的
inevitably,adv.,不可避免地，必然地
inextricably,adv.,无法摆脱地
infancy,n.,幼年
infection,n.,传染（病）
infectious,adj.,传染性的，易传染的
inferior,adj.,（等级、社会地位、质量等）差的；（较）低的
inferiority,n.,自卑感
infertile,adj.,不肥沃的
infest,vt.,骚扰，大批滋生
infiltrate,vt.,使透过，渗透
inflate,vt.,使膨胀，使充气
inflation,n.,通货膨胀
inflexible,adj.,坚定的
inflict,vt.,造成；(使)遭受(痛苦、损伤等)
afflict,vt.,（外来到）折磨
influential,adj.,有影响的；有权势的
influenza,n.,流行性感冒
influx,n.,流入，汇集
informal,adj.,不拘礼节的，随便的；非正式的
informed,adj.,受过教育的，见多识广的
be informed about,词组,熟悉
infuse,vt.,注入，灌输
infusion,n.,注入
ingenious,adj.,机灵的，有独创性的
ingeniously,adv.,天才地，独创地
ingredient,n.,配料，成份
inhabit,vt.,居住于，栖息于
habitat,n.,栖息地，居留地
inhabitant,n.,居民，住户
inherent,adj.,固有的，内在的
inherit,vt.,继承
inheritance,n.,遗产
inhibit,vt.,抑制；阻止
initiate,vt.,开始，发起
initial,adj.,最初的，初始的
initial,n.,开头大写字母
initially,adv.,最初，开头
inject,vt.,注射；注入，灌溉
innate,adj.,先天的，天生的
innermost,adj.,最里面的；内心的，秘密的
innocent,adj.,无辜的，清白的；天真的
innovation,n.,改革，创新
innovative,adj.,创新的，革新的
innumerable,adj.,无数的，数不清的
inorganic chemical,词组,无机化合物
inquiry,n.,质问；调查
insanity,n.,精神错乱，疯狂
insert,vt.,插入
insight,n.,洞察力，见识
insist,vt.,坚持认为；坚决主张
insistence,n.,坚持
inspect,vt.,检查；审查
inspection,n.,检查，细看
inspector,n.,检查员，巡视员
inspire,vt.,鼓舞；给...以灵感
inspiring,adj.,使人振奋的，鼓舞的
inspiration,n.,灵感
install,vt.,安装，设置
installment,n.,分期付款
instantaneous,adj.,瞬间的，即刻的
instinct,n.,本能，直觉
instinctive,adj.,天生的，本能的
instinctively,adv.,本能地；凭直觉地
institute,n.,学院；学会，协会
institute,vt.,开始；制定
institution,n.,机构；惯例
institutionalize,v.,使制度化
instructive,adj.,有益的；教育性的
instruct,vt.,教；命令
instruction,n.,命令，指示；教学
instrument,n.,仪器；工具；乐器
instrumental,adj.,有帮助的，可作为手段的
instrumental,adj.,器具的
instrumentalist,n.,器乐家，器乐演奏者
insulation,n.,绝缘；隔离；孤立
insulin,n.,胰岛素
insult,vt.,侮辱，凌辱
insulting,adj.,侮辱性的，欺负人的
insurmountable,adj.,不能克服的，不能超越的
intact,adj.,尚未被人碰到的，完整的
integral,adj.,整体的；构成整体所需的
integrate,vt.,使成为整体，使一体化
integrated,adj.,整合的，一体化的
disintegrate,v.,（使）分解，（使）碎裂
integrity,n.,完整性；正直，诚实
intellect,n.,智力
intellectual,n.,知识分子 adj. 智力的
intelligent,adj.,聪明的，有才智的
intelligence,n.,智力；聪明
intelligible,adj.,明了的，可理解的
intense,adj.,强烈的，剧烈的
intensify,vt.,加强；扩大
intensive,adj.,加强的；集中的，密集的
intentionally,adv.,有意地，特意地
intention,n.,意图，目的=intent
interaction,n.,相互作用
interact,vi.,互相作用，互相影响
interactive,adj.,交互式的
interconnected,adj.,相互连接的
interconnecting,adj.,相互连接的
interdependence,n.,互相依赖
interdependent,adj.,相互依赖的，互助的
interest,n.,兴趣；利息
interfere,vi.,妨碍；干涉
interference,n.,冲突；干涉
interior,n./adj.,内部（的）
interlocking,adj.,连锁的，关联的
intermediate,adj.,中间的，中级的
intermittent,adj.,间歇的，断断续续的
internal,adj.,内在的
external,adj.,外部的
interpersonal,adj.,人与人之间的；关于人与人之间关系的
interpretation,n.,解释
interrupt,vt.,打断，使中断
intersect,vi.,相交
intersection,n.,十字路口，交叉点
interstellar,adj.,星际的
interval,n.,间隔时间
intervention,n.,干涉，介入
intimate,adj.,亲密的
intimate,adj.,（知识）渊博的
intimacy,n.,亲密，熟悉
intoxication,n.,陶醉，醉酒
intrepid,adj.,勇敢的
intricate,adj.,复杂的，错综的
intricately,adv.,杂乱地
intrigue,vt.,激起...的兴趣
intriguing,adj.,吸引人的,引起兴趣(或好奇心)的
intrinsic,adj.,固有的，内在的
introspective,adj.,自省的，反省的
intrusion,n.,侵扰，干扰
intruder,n.,入侵者，闯入者
intruding,adj.,入侵性的
inundate,vt.,淹没
invade,vt.,侵略，侵犯
invasion,n.,入侵
invader,n.,侵略者
invariably,adv.,不变地，总是
inventory,n.,存货清单；库存品
invertebrate,n./adj.,无脊椎动物（的）
vertebrate,n./adj.,脊椎动物（的）
investigate,vt.,调查，研究
investigation,n.,调查
inviting,adj.,诱人的，引人心动的
involuntary,adj.,不知不觉地，无意识的
ion,n.,离子
irony,n.,反讽，讽刺
ironic,adj.,讽刺的
irregular,adj.,不规则的，不整齐的
irregularly,adv.,不规则地
irrelevant,adj.,离题的；无关的
irreparable,adj.,不能挽回的
irresistible,adj.,无法抗拒的,无法抵抗的;诱人的
irresponsible,adj.,不负责的
irreverent,adj.,不敬的
irreverence,n.,不敬；不敬的行为
irreversible,adj.,不可改变的；不可撤销的
irrevocable,adj.,不能取消的
irrigate,vt.,灌溉
irrigation,n.,灌溉
irritate,vt.,激怒；使过敏
irritable,adj.,易怒的，急躁的
irritating,adj.,刺激的；使人愤怒的，气人的
isolate,vt.,使隔离，使孤立
isolated,adj.,与世隔绝的，偏僻的；孤独的
isolation,n.,隔离；孤立
issue,n.,问题
issue,n.,（报刊的）期
issue,v.,发行
itinerary,n.,路线
itinerant,adj.,巡回的
jar,n.,罐，广口瓶
jar,vt.,震动；刺激，震惊
ajar,adj.,（门窗等）微开的
jealousy,n.,妒忌，羡慕
jelly,n.,果冻，胶状物
jellyfish,n.,水母
jewel,n.,宝石
jewelry,n.,珠宝
jeweler,n.,宝石商，宝石匠
jibe with,词组,与...一致
jog,vi.,慢跑
jogging,n.,慢跑
jolt,n.,震动，颠簸
jolt,vt.,使颠簸，猛击
jolting,adj.,令人震惊的
journal,n.,期刊，杂志
journal,n.,（航海）日记；分类帐
journalist,n.,新闻记者
journalistic,adj.,新闻事业的，新闻工作的
journalism,n.,新闻业, 报章杂志
jubilant,adj.,欢腾的，喜气洋洋的
judgement,n.,判断
judgement,n.,判决
judicious,adj.,明智的；有判断力的
juice,n.,（水果）汁，液
juicy,adj.,多汁液的
jumble,vt.,使混乱，混杂
jumble,n.,混乱；杂乱的一堆
junction,n.,连接；汇合处
juncture,n.,接合，接缝
jungle,n.,丛林
jurisdiction,n.,权限；管辖范围
jury,n.,陪审团
injury,n.,伤害，侮辱
judgement,n.,判决
verdict,n.,[律](陪审团的)裁决
sentence,n./v.,宣判
justify,vt.,证明...是正当的或有理的；为...辩护
justice,n.,正义，公平，公正
injustice,n.,不公平，不讲道义
justly,adv.,公正地，正当地
juvenile,n./adj.,青少年（的）
juxtaposition,n.,毗邻，并置，并列
keen,adj.,（感觉、观察、理解等）敏锐的，敏捷的
keen,adj.,锋利的
ken,n.,视野，知识领域
kennel,n.,狗舍，狗窝
kernel,n.,仁，核心；(去壳的)麦粒，谷粒
kerosene,n.,煤油
gasoline,n.,汽油
kiln,n.,（砖、石灰等）窑，炉
kinetic,adj.,运动的；动力学的
kingdom,n.,王国
laborious,adj.,费力的，艰难的
laboriously,adv.,费力地，艰难地
laborer,n.,劳动者
labor movement,词组,劳工运动
labor union,词组,工会
labyrinth,n.,迷宫；错综复杂的事件
lace,n.,饰带，花边
lace,n.,带子；鞋带
lace,vt.,装饰，点缀
lag,vi.,落后
lament,vt.,为...悲痛，痛惜
landing,n.,着陆，降落
landmass,n.,大片陆地
landscape,n.,风景，山水；风景画
landslide,n.,山崩
largely,adv.,大量地，大半地
larvae,n.,幼虫
larynx,n.,[解] 喉
laser,n.,激光
lash,n.,鞭子
lash,v.,抽打，鞭打
splash,v.,溅，泼
flash,n./vi.,闪光；闪现
lasting,adj.,持久的；永恒的，耐力的
long-lasting,adj.,持久的，永久的
everlasting,adj.,永恒的，持久的，无止境的
latent,adj.,潜伏的，隐藏的
lateral,adj.,侧面的，旁边的
equilateral,词组,等边三角形
quadrilateral,词组,四边形
bilateral,adj.,有两面的, 双边的
latitude,n.,纬度，范围
latitude,n.,行动或言论的自由（范围）
platitude,n.,陈词滥调
launch,vt.,发动
launch,n./v.,发射，（使）升空
laundry,n.,洗衣店, 要洗的衣服
laurels,n.,桂冠，荣誉
lava,n.,熔岩，岩浆
volcano,n.,火山
lave,vt.,为沐浴, 洗
lavish,adj.,过分丰富的；浪费的
slavish,adj.,卑屈的；盲从的
lawn,n.,草地，草坪，草场
lawsuit,n.,诉讼
lay,vt.,产卵，下蛋
layer,n.,层，层次；阶层
layering,n.,分层堆积
layman,n.,俗人，犯人；外行
layout,n.,规划，设计
leading,adj.,最主要的
leading,adj.,主演的
mislead,vt.,误导
leadership,n.,领导能力，领导阶层
league,n.,同盟，联盟；联合会
colleague,n.,同事，同僚
leak,vt.,漏；渗
leak,vt.,泄露
leaky,adj.,漏的
bleak,adj.,荒凉的，凄凉的
leap,n./vi.,跳跃
learn by rote,词组,死记硬背学习
lease,n.,租约
lease,vt.,出租，租借
ledge,n.,壁架, 架状突出物, 暗礁, 矿层
pledge,n.,誓言 vt. 保证
legacy,n.,遗产，遗物
heritage,n.,遗产；继承物
legendary,adj.,传奇的
legend,n.,传说；传奇文学
legible,adj.,(指印刷或字迹)清楚地，易读的
illegible,adj.,难辨认的，字迹模糊的
legislation,n.,立法
legislation,n.,法规
legislate,vt.,立法
legislature,n.,立法机关
legume,n.,豆类，豆荚
leisure,n.,空闲，闲暇
leisurely,adj.,从容的，悠闲的
lengthen,vt.,延长，使变长
lengthy,adj.,(演说、文章等)冗长的，啰嗦的
leopard,n.,豹，美洲豹
lethal,adj.,致命的
lethargic,adj.,昏睡的；懒洋洋的，萎靡不振的
lethargy,n.,昏睡，倦怠
lettuce,n.,莴苣，生菜
liable of,词组,倾向于
libel,n.,以文字损害名誉，诽谤
liberal,adj.,开放的,主张变革的,自由主义的
liberal,adj.,慷慨的，不拘泥的
liberate,vt.,释放，使自由
liberty,n.,自由
liberally,adv.,随意地，不受限制地
deliberate,adj.,深思熟虑的；故意的
life span,词组,寿命
lift,v.,举起，升高
facelift,n.,改建，表面修改
light bulb,词组,灯泡
bulb,n.,电灯泡；球茎
likewise,adv.,同样地，也
limb,n.,肢，翼；树枝
limestone,n.,石灰石
lineage,n.,血统，世系
linen,n.,亚麻布，亚麻制品
linger,vi.,逗留；徘徊
linguistic,adj.,语言上的，语言学上的
linguistics,n.,语言学
liquid,n.,液体，流体
liquid,adj.,液体的
literally,adv.,照字面意义地，逐字地
literary,adj.,文学（上）的，从事写作的
literate,adj.,有文化的，有阅读和写作能力的
literacy,n.,有读写能力，识字
illiterate,adj.,不识字的，没受教育的
semiliterate,n.,半文盲，有初等文化者
obliterate,vt.,涂去，删除
iterate,vt.,反复说，重申
litter,vt.,乱丢
litter,vi.,乱丢垃圾
litter,n.,垃圾
livelihood,n.,生计，谋生
lively,adj.,活泼的，活跃的；栩栩如生的
liver,n.,肝脏
livestock,n.,家畜，牲畜
living quarters,词组,住房，住处
load,n.,重担；重任
load,vt.,装载，使负担
unload,vt.,摆脱...的负担，卸
overload,vt.,使超载，超过负荷
shipload,n.,船货, 一船上的载货
trainload,n.,列车载重
workload,n.,工作量
loathsome,adj.,令人讨厌的
lobby,n.,大厅，休息厅
lobby,v.,游说议员
lobster,n.,龙虾
locate,vt.,找出，定位；设置
location,n.,位置，场所
locality,n.,位置，地方
locally,adv.,在地方上，在当地
locomote,v.,移动，行动
locomotion,n.,运动，移动
locomotive,n.,火车头
lodge,n.,（山林）小屋
lodge,vi.,寄存；临时住宿
lodging,n.,寄宿，住房（常指出租的房间）
log,n.,圆木；航海日志；vt. 记录
log cabin,词组,小木屋
logical,adj.,合乎逻辑的，合理的
logic,n.,逻辑，逻辑学
long-range,词组,远程的
longevity,n.,长寿
longitude,n.,经度，经线
loom,n.,织布机
loom,vi.,隐约闪现，逼近
bloom,v.,（使）开花，（使）繁盛 n. 花
loop,vt.,把...圈成环；缠绕
loop,n.,圈，环
sloop,n.,单桅帆船
loose,adj.,宽松的；不牢固的
loosen,vt.,解开，放松
lore,n.,口头传说；学问，知识
lucrative,adj.,赚钱的，有利可图的
lull,vt.,使平静；使安静
lumber,n.,木材，木料
lumbering,n.,采伐林木
timber,n.,木材，木料
slumber,n.,睡眠
plumber,n.,水管工人
luminosity,n.,亮度，发光度
luminous,adj.,发光的；明亮的
lunar,adj.,月的
lure,vt.,引诱，诱惑
lure,n.,饵；诱惑
luster,n.,光彩，光泽；声望
lustrous,adj.,有光泽的；光辉的
luxurious,adj.,奢侈的，豪华的
luxury,n.,奢侈，豪华
lyric,n.,抒情诗；歌词
lyricist,n.,抒情诗人
lyrically,adv.,抒情地
magic,n.,魔法，魔力；魔术
magical,adj.,魔术的，不可思议的
magician,n.,魔术师，变戏法的人
magnesium,n.,镁
magnetic,adj.,磁的，有磁性的
magnetic,adj.,有吸引力的
magnetic,n.,磁铁
magnetism,n.,磁，磁力
magnetize,vt.,使磁化；吸引
demagnetize,vt.,消磁，使退磁
ferromagnetic,adj.,铁磁的, 铁磁体
geomagnetic,adj.,地磁的
electromagnetic,adj.,电磁的
magnificent,adj.,华丽的；高尚的；丰富的
magnificence,n.,华丽，富丽堂皇
magnify,vt.,放大，扩大
magnitude,n.,大小，光度，重要
maintenance,n.,维修，保养
maintenance,n.,维持，继续
maize,n.,玉米
corn,n.,玉米，谷类
popcorn,n.,爆米花
cornet,n.,短号
majestic,adj.,壮观的，庄严的
majesty,n.,威严，最高权威
majority,n.,多数
makeup,n.,组成，结构
makeup,n.,补考
makeup,n.,化妆品
malfunction,n.,故障，失灵
mall,n.,大规模购物中心
mallet,n.,槌棒
malleable,adj.,可锻的，可塑的，可延展的
malleability,n.,可锻性，可塑性，延展性
malnutrition,n.,营养不良
mammal,n.,哺乳动物
mammoth,n.,猛犸象
mammoth,adj.,巨大的
mandatory,adj.,命令的，强制的
mandate,n.,命令，要求
maneuver,vt.,(敏捷地)操纵；(用策略)调动
maneuver,n.,策略，谋略
mania,n.,狂热
manifestation,n.,表现，显示
manipulate,vt.,操作；操纵，利用
mannerism,n.,特殊习惯，怪癖
mansion,n.,大厦；公寓
mantle,n.,地幔；覆盖物
manual,n.,手册
manual,adj.,人工的，手动的
manually,adv.,用手
manure,n.,肥料
map,vt.,绘制...的地图
marble,n.,大理石
march,vi./n.,前进，行进
margin,n.,边缘；余地
marine,adj.,海的，航海的，海产的
aquamarine,n.,绿玉；碧绿色
submarine,n.,潜水艇，潜艇
maritime,adj.,海上的，海事的
markedly,adv.,显著地，明显地
marrow cavity,n.,骨髓腔
marsh,n.,湿地，沼泽
marshy,adj.,沼泽的
marvel,n.,奇异的事物，罕见的例子
marvelous,adj.,不可思议的；了不起的
mask,n.,面具
mask,v.,掩盖，掩饰
mason,n.,泥瓦匠
stonemason,n.,石工，石匠
masonry,n.,石工术；石匠职业
massive,adj.,大而重的，庞大的；可观的
masterpiece,n.,杰作，名著
matching,adj.,(尤指颜色或外表)相配的,一致的
matchless,adj.,无敌的；无比的
mate,v.,结伴，成配偶
mate,n.,配偶
mathematics,n.,数学
maverick,adj.,特立独行的；不遵守传统的
maverick,n.,持异议者，自行其是者
meager,adj.,缺乏的，不足的
meaningful,adj.,意味深长的
measures,n.,措施，方法
mechanical,adj.,机械的；机械性的，呆板的
mechanics,n.,机械学，力学
mechanized,adj.,机械化的
mechanism,n.,机械；机理，运作机制
media,n.,媒体
mediate,v.,斡旋，调停
intermediate,adj.,中间的；中级的
mediator,n.,调停者
medieval,adj.,中世纪的
medium,n.,媒体，媒介；方法
melodious,adj.,旋律优美的，悦耳的
melody,n.,悦耳的音调
melodic,adj.,有旋律的, 调子美妙的
melodrama,n.,音乐剧；情节剧
membrane,n.,膜，薄膜
memo,n.,备忘录
memory,n.,记忆，记忆力；回忆
memorize,vt.,纪念
memorable,adj.,难忘的
commemorate,v.,纪念
memorandum,n.,备忘录, 便笺
memorial,n.,纪念物
menace,n.,危险物
menace,vt.,威吓；胁迫
mend,vt.,修补；改进
amendment,n.,改善，改正
recommend,vt.,推荐；介绍
mention,vt.,提及；说起
menu,n.,菜单；选择单
mercantile,adj.,商业的，贸易的
merchant,n.,商人
merchandise,n.,商品
merge,v.,（使）合并，（使）融合
merger,n.,合并
emerge,vi.,显现，浮现
emergence,n.,浮现，出现
meridian,n.,子午线
merit,n.,优点，价值
mesmerize,vt.,施催眠术；使入迷，迷住
messy,adj.,肮脏的；凌乱的
metabolism,n.,新陈代谢
metabolic,adj.,新陈代谢的
metaphor,n.,隐喻，暗喻
meteor,n.,流星
meteorite,n.,陨星，陨石
meteorology,n.,气象学
meteorologist,n.,气象学者
methodology,n.,方法论，方法学
methodically,adv.,有条理地；井然地
meticulous,adj.,一丝不苟的，过细的
metropolitan,adj.,主要都市的，大城市的
microbe,n.,微生物
microcosm,n.,微观世界
microorganism,n.,微生物
microscope,n.,显微镜
microscopic,adj.,用显微镜可见的；极小的
telescope,n.,望远镜
microwave,n.,微波（炉）
mighty,adj.,强大的，巨大的
migrant,n.,候鸟；移民
emigrant,n.,移居外国者，移民
immigrant,n.,（从外国移入的）移民，侨民
migrate,vi.,移动，迁徙
migration,n.,定期迁移；迁居
migratory,adj.,迁移的；流浪的
milestone,n.,里程碑；划时代的事件
milieu,n.,环境；出身背景
militant,adj.,好战的，好暴力的
millennia,n.,一千年，千禧年
millionaire,n.,百万富翁
mime,vt.,模拟，模仿
mimetic,adj.,模仿的，（生物）拟态的
mimicry,n.,（生物）拟态；模仿
mimic,n./vt.,模仿，摹拟
mingle,v.,（使）混合
mingling,adj.,混合的
miniature,adj.,微型的，缩小的
miniature,n.,缩小的模型，缩图，缩影
minimize,v.,最小化，减到最小
minimal,adj.,最小的，最少的
minimalist,n.,最低限要求者
minimum,adj.,最小的，最低的
minimum,n.,最小值，最小化
minority,n.,少数；少数民族
minuscule,adj.,极小的
minute,adj.,微小的
miraculous,adj.,奇迹的，不可思议的
miracle,n.,奇迹，奇事
mirage,n.,海市蜃楼，幻景
miserable,adj.,痛苦的，悲惨的，可怜的
mislead,vt.,使误入歧途，误导
misleading,adj.,使人误解的；骗人的
mistrust,vt.,不信任，不相信，怀疑
mistrustful of,词组,不相信，怀疑
mitigate,vt.,使减轻，使缓和
moat,n.,护城河，壕沟
mobile,adj.,可移动的
mobile,adj.,易变的
mobility,n.,流动性；灵活性
immobile,adj.,稳定的，不动的，静止的
mock,v.,（模仿性的）嘲笑
mockingbird,n.,（动物）仿声鸟
moderate,adj.,温和的；适度的，有节制的
moderate,vt.,使缓和，使稳定
modest,adj.,谦虚的；适度的
modify,vt.,修改，变更
moist,adj.,潮湿的，多雨的
moisten,vt.,弄湿，使湿润
moisture,n.,潮湿，湿气；湿度
mold,vt.,塑造
mold,n.,模子；铸型
mold,n.,霉菌
molecule,n.,分子
molecular,adj.,分子的
molten,adj.,熔融的，熔化的
monarch,n.,君主，帝王
monetary,adj.,货币的，金钱的
monitor,vt.,监控；监测
monochromatic,adj.,单色的，一色的
monopolize,vt.,独占，垄断
monopoly,n.,独占，垄断
monotonous,adj.,单调乏味的，无变化的
monster,n.,怪物，巨兽
monument,n.,纪念碑；纪念物
mounmental,adj.,杰出的，不朽的
morale,n.,士气
morphololgy,n.,形态学，形态论
mortal,n.,凡人，人类
mortal,adj.,必死的；致命的
most distinctive,词组,最杰出的，典型的
motif,n.,（作品）主题，主旨
motion,n.,运动
motionless,adj.,不动的，静止的
locomotion,n.,运动，移动
motivate,vt.,激发，刺激
motivation,n.,动机，刺激
motive,n.,动机
locomotive,n.,火车头
automotive,adj.,汽车的，自动推进的
mourning,n.,悲恸，服丧
mournful,adj.,悲痛的，悲哀的
mouth,n.,入口
mud mortar,词组,灰泥浆
mortar,n.,灰泥；臼, 研钵
mulberry,n.,桑树
multicellular,adj.,多细胞的
multi-faceted,adj.,多方面的；多才多艺的
multiple,adj.,多样的，多重的
multiple,n.,倍数
multiply,v.,繁殖，增加
multiply,v.,乘（by）
multistory,n.,多层建筑
multitude,n.,大量，多数
mundane,adj.,世俗的，平凡的
mundane,adj.,平常的，普通的
municipal,adj.,市的，市政的
mural,n.,壁画，壁饰
muscular,adj.,强壮的，肌肉发达的
mushroom,n.,蘑菇
mushroom,v.,迅速生长
mutual,adj.,相互的；共有的
myriad,n.,许多，无数
myriad,adj.,无数的
mysterious,adj.,神秘的
myth,n.,神话，神话故事
mythical,adj.,神话的，虚构的
mythology,n.,（总称）神话
"""

def parse_word_data(data):
    """
    解析原始字符串数据，提取单词和中文释义。
    :param data: 原始字符串数据
    :return: 包含 {word: definition} 的字典
    """
    word_dict = {}
    lines = data.strip().split('\n')
    
    # 用于合并跨行定义的逻辑
    current_word = None
    
    for line in lines:
        line = line.strip()
        if not line:
            continue
            
        # 尝试使用逗号分割，但考虑到有些释义中包含逗号，采取更健壮的策略
        # 我们假设英文单词（可能是词组）在第一个逗号之前，词性在第二个逗号之前，释义在之后
        parts = [p.strip() for p in re.split(r',', line, maxsplit=2)]
        
        if line.startswith(','):
            # 这是跨行或附加的定义（如: ,vt.,使逐步上升）
            # 找到前一个单词的释义，并追加（去重）
            if current_word and len(parts) >= 3:
                # 重新组合释义部分
                new_definition = parts[2].strip().strip('"')
                if new_definition and new_definition not in word_dict[current_word]:
                    word_dict[current_word].append(new_definition)
            elif current_word and len(parts) == 2: # 只有词性和释义
                new_definition = parts[1].strip().strip('"')
                if new_definition and new_definition not in word_dict[current_word]:
                    word_dict[current_word].append(new_definition)
            continue
        
        # 正常行 (erratic,adj.,古怪的，反复无常的)
        if len(parts) >= 3:
            word = parts[0].strip()
            definition = parts[2].strip().strip('"')
            
            # 清理定义中的引号
            definition = definition.replace('"', '') 
            
            # 词组和单词的定义可能需要分开处理，但为简化，我们尝试提取所有中文部分
            chinese_parts = re.split(r'[;；]', definition) # 按分号分割可能的多个释义
            
            # 过滤掉空的释义
            valid_definitions = [p.strip() for p in chinese_parts if p.strip()]
            
            if word and valid_definitions:
                word_dict[word] = valid_definitions
                current_word = word
            
    
    # 最终处理：将所有可能的中文释义合并成一个字符串
    final_word_dict = {}
    for word, def_list in word_dict.items():
        # 合并所有释义，用中文顿号隔开
        final_word_dict[word] = "、".join(def_list)

    return final_word_dict

def get_quiz_data(word_dict):
    """
    将字典转换为测验题目列表，每题包含英文单词和正确中文释义。
    :param word_dict: {word: definition} 字典
    :return: 包含 [{'word': str, 'correct_def': str}] 的列表
    """
    quiz_data = []
    for word, definition in word_dict.items():
        quiz_data.append({'word': word, 'correct_def': definition})
    return quiz_data

def generate_options(correct_def, all_definitions, num_options=4):
    """
    为题目生成4个选项，包含一个正确答案和3个随机的错误答案。
    :param correct_def: 正确答案的中文释义
    :param all_definitions: 所有单词的中文释义列表
    :param num_options: 选项数量
    :return: 随机排列后的选项列表
    """
    # 1. 收集所有不重复的错误释义
    incorrect_defs = list(set(all_definitions) - {correct_def})
    
    # 2. 随机选择3个错误答案 (如果不足3个，则使用所有可用的错误答案)
    num_incorrect = min(num_options - 1, len(incorrect_defs))
    if num_incorrect < num_options - 1:
        # 如果可用的错误选项不足，则重复使用
        if incorrect_defs:
             # 如果有错误选项，循环重复直到达到选项数量
            chosen_incorrect = random.choices(incorrect_defs, k=num_options - 1)
        else:
             # 如果没有其他选项，则只能使用正确答案
            chosen_incorrect = []
            
    else:
        chosen_incorrect = random.sample(incorrect_defs, num_options - 1)
    
    # 3. 组合选项并打乱
    options = [correct_def] + chosen_incorrect
    random.shuffle(options)
    
    return options

def run_quiz(quiz_list, word_definitions):
    """
    运行单词测验，直到所有题目都答对为止。
    :param quiz_list: 待测验的题目列表
    :param word_definitions: 所有中文释义的列表
    """
    
    # 待回答正确的题目列表，初始包含所有题目
    questions_to_answer = list(quiz_list) 
    
    # FIX: 将集合元素改为可哈希的单词字符串，而非不可哈希的字典对象
    # 记录错误题目的集合，用于在循环中检查并提示。存储的是单词字符串 (word)。
    incorrect_words = set() 
    
    total_words = len(questions_to_answer)
    questions_attempted = 0
    
    print("=" * 40)
    print(f"  Word Quiz - 共 {total_words} 个单词")
    print("  请选择正确的中文释义。")
    print("=" * 40)
    
    while questions_to_answer:
        # 随机选择一个题目进行测试
        current_question = questions_to_answer.pop(0) 
        
        word = current_question['word']
        correct_def = current_question['correct_def']
        
        # 生成选项
        options = generate_options(correct_def, word_definitions, num_options=4)
        
        questions_attempted += 1
        
        # 显示题目
        # FIX: 使用 word 字符串检查是否是错误重现的题目
        is_retry = word in incorrect_words
        status_text = "🔄 重试" if is_retry else f"待完成: {len(questions_to_answer) + 1} / 总词汇量: {total_words}"
        print(f"\n--- 第 {questions_attempted} 题 ({status_text}) ---")
        print(f"英文单词：{word}")
        
        # 打印选项
        option_map = {}
        for i, opt in enumerate(options):
            letter = chr(ord('A') + i)
            option_map[letter] = opt
            print(f"  {letter}. {opt}")
            
        # 用户输入
        while True:
            user_input = input("请选择 (A/B/C/D) 或输入 'exit' 退出: ").strip().upper()
            if user_input == 'EXIT':
                print("\n测验已中止。下次再来挑战吧！")
                return
            if user_input in option_map:
                break
            print("输入无效，请重新输入 A, B, C, D 或 exit。")

        
        # 检查答案
        selected_def = option_map[user_input]
        
        if selected_def == correct_def:
            print(f"✅ 回答正确！{word} 的释义是: {correct_def}")
            # FIX: 如果之前答错了，现在答对了，则从错误集合中移除该单词字符串
            if is_retry:
                incorrect_words.remove(word)
        else:
            print(f"❌ 回答错误。正确答案是: {correct_def}")
            # 将答错的题目添加到队列末尾，以便再次出现
            questions_to_answer.append(current_question)
            # FIX: 将答错的单词字符串添加到错误集合中
            incorrect_words.add(word)

    print("\n" + "=" * 50)
    print("🎉 恭喜！所有单词都已回答正确！你真棒！ 🎉")
    print(f"总共尝试了 {questions_attempted} 次，学习愉快！")
    print("=" * 50)


# --- 主程序逻辑 ---

if __name__ == "__main__":
    # 1. 解析数据
    parsed_words = parse_word_data(WORD_DATA)
    
    if not parsed_words:
        print("未找到有效的单词数据。请检查文件格式。")
    else:
        # 2. 准备测验数据
        quiz_data = get_quiz_data(parsed_words)
        
        # 3. 准备所有中文释义的列表，用于生成错误选项
        all_definitions = list(parsed_words.values())
        
        # 4. 随机打乱初始顺序
        random.seed() # 使用当前时间作为随机种子
        random.shuffle(quiz_data)
        
        # 5. 运行测验
        run_quiz(quiz_data, all_definitions)