sdcache接口说明
 该项目的核心功能是对用户的url进行缓存，以避免web服务器对相同url频繁的计算，减少web服务器压力
 同时节约客户端的访问时间。sdcache基于http协议，单键值储存，最大可支持压缩后1M的数据
 数据压缩是指目标网页将使用zlib.compress方法进行压缩，不要担心，解压工作sdcache已经在返回之前做完了。
 
sdcache使用无需安装，直接在你喜欢的目录下解压，make&run

sdcache使用http协议访问，参数简单，你只需要传入
 url：     需要访问的目标url，请注意使用urlencode
 view:     当view=1时，将默认打印每个线程的等待队列情况，以方便对服务压力情况进行即时反馈。
 debug:    当debug=1时，配合不同的url，将打印出不同线程目前的cache使用情况
    debug信息示例:
       thread 7//线程序列号
       bucket_num:524288 //hashmap分配了多少桶
       value_size:3000//当前设置的存储值最小单元（1 chunk，每个value由n(n>=1)个chunk通过链表组成）
       slice_size:131072//slice使用了多少chunk
       slice_count:1//分配了多少slice（slice是内存池分配的最小单位）   
       items://存储了多少条目
       chunk_cnt:0//使用了多少chunk.item与chunk的关系是1<->N
       free_cnt: 0//有多少chunk已经被回收    
       max_size: 2048M//最大内存
       alloc_size:381.000000M//当前已经使用的内存
       
      
sdcache使用说明
第一步: 下载sdcache源码 
  git clone https://github.com/sheldwu/sdcache/

第二步: 编译可执行程序
  cd sdcache/server; make
  
启动及停止sdcache
sdcache -n 1 -p 8390 -e 6000 -s 100 -m 2000 -b 1000 -k start -v 10000
  -p listening_port: 监听的端口号
  -e expire_time: 缓存失效时间 （default:1Year）
  -s slice_size:每片的大小，建议设置在与数据规模1/10左右（defalut:100000）
  -m max_size:内存使用的最大限制,单位为M（default:2048M）
  -b bucket_number: 桶数, 4至200000000之间的整数, 不宜过大过或过小, 以数据规模120%为宜. 过大会造成空间浪费, 过小则会降低存取速度.（default:120000 ）
  -k start|stop|restart: start为启动, stop为停止,restart为重启
  -v value_size：因sdcache数据存储采用分段切割存储，value_size为每段数据的大小，设置应该比数据集中长度区稍大一些（如数据长度热点是1000，则最好设置成1200），过小会导致数据切割过多，造成反复寻址，浪费时间，过大则空间浪费严重(default:1000)


运作原理 
 sdcache是支持单键获取的缓存系统， 单键存取是指在插入数据时, sdcache在一次HTTP连接中接受一个键(url)，在返回数据时，首先在自身缓存池中查找，若没有命中，则从目标url自动获取并进行存储并返回给用户.
 ps，若目标数据压缩后大于最大缓存长度，则该数据存储时会被抛弃，但仍可以正常返回，此时sdcache退化为代理

输入输出说明
  1. 有以下两点需要说明
    1.1 使用时一次只能访问一个url，并且该url必须进行urlencode. 
    1.2 url支持http协议
      http://localhost/?params


为什么使用ddcache而不是memcache

1.ddcache使用更简单，轻量级的应用删除了客户端的概念，如果你的配置文件是tart_url = abc.com，那么现在只需要改成tar_url = ddcache.com?url=abc.com，就可以方便的使用ddcache，没有客户端的概念，可以简单的ddcache当作代理缓存使用，只不过我们不在硬盘而是内存存储数据。 li>

2. memcache设计的多适应性，因此需要手动get,set键值对，而ddcache只是为url缓存设计，用户只需要get即可，若未命中，ddcache将自动从url抓取并存储之后，所有的操作对用户透明，无须用户二次写入。并且memcache采取的lru-k算法，虽然降低了缓存污染的概率，但是由于算法本身的复杂性，需要维护一张节点访问次数历史表，花费一定空间与时间，lru-1虽然在面对偶发，周期性数据的批量数据，命中率较低，但在url环境下，我们更多应该考虑的是热点数据，无需使用空间与时间去换取有限的命中率。

3. memcache内存池的分配规则，导致其在存储值的长度分布较为散列的数据时，数据丢失率增加，而ddcache因为对于不定长数据采取的是分割法，即用若干个定长（value_size）的数据，采用链表挂接的方法存储，因此在存储数据长度散列较为明显的情况下，也能保持高命中率。

4. memcache由于要求服务场景的多适应性，设计了较为冗余的锁机制，具体如下
a. slabs lock全局锁，每次内存管理模块申请，释放内存，slab维护线程工作都需要获取slabs lock；
b. stats lock数据统计锁，每个工作线程各有单独的stats lock；
c. cache lock全局锁，内存中hash表的操作需持有该锁；
d. item lock锁，memcache创建大量的item lock锁，对k-v操作时，对k进行hash,然后取对应的item lock锁
其中：

a、b、d都是由于多个线程共享数据造成的
c是由于memcache的数据统计需要而产生
可以看到大多数锁都是由于多线程共享数据造成的，因此，如果线程单独享有一份数据，将大大减少锁的使用
ddcache正是采用的线程独享数据，每一个url被hash到不同的线程中，每个线程的数据各自独立，因此虽然服务器是多线程处理，但是每个数据只会被一个线程处理，而且并不会产生冗余数据

对比测试
  测试数据集
    将线上某台机器搜索日志107万进行顺序访问，搜索词58万，其中高命中率关键词（搜索次数大于等于10次）6631（大小为120M），搜索总数为35万，其余65万次访问，为低命中率关键词 memcache与ddcache分别使用120M最大空间，查看命中率情况
  测试参数
    ddcache启动参数ddcache -n 4 -p 8390 -v 1500 -s 4000 -b 600000 -k restart -m 30
    memcache 启动参数 memcached -d -m 120（默认4个线程）

  测试结果
    缓存/对比点	  遍历时间	命中次数	命中率	内存分配	实用内存
      memcache	    13524s	368473    	34%   	122M  	106M
      ddcache	      11402s	356007	    33%	    120M	  119M

备注
内存分配是指cache已经分配了的空间，实用内存是指已经被用来做缓存数据的内存，在这一块上因为memcache的按value长度分配slab会导致其会有一部分闲置空间无法使用
结果分析

可以看到ddcache因为减少了多线程锁，遍历时间会小于memcache,由于memcache使用的lru-k k>=1算法
所以命中率稍微高些，但是可以看到提升并不是特别明显，内存分配如预期利用率偏低
