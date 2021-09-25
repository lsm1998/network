# GORM技术分享-入门篇

## 1.为什么要使用GORM

对比现有方式的优点：

1. 统一的读写分离配置，无须每次手动指定读库写库、轮询等；

2. 自动化的增、删、改；

3. 功能更强的select，包括join和对结果集的处理；

4. 更强大的占位符；

5. 更便利的查询条件拼接；

6. 使用钩子函数，新增、修改时间无需每次手动赋值；

7. ...

   

## 2.如何使用GORM

**1.定义数据库模型**

````Golang
type Student struct {
	Id       int64 `gorm:"primarykey;autoIncrement"`
	Name     string
	Age      uint32
	Birthday time.Time `gorm:"name:time"`
}

func (*Student) TableName() string {
	return "t_student"
}

type Course struct {
	Id        int64 `gorm:"primarykey;autoIncrement"`
	StudentId int64
	Name      string
}

func (*Course) TableName() string {
	return "t_course"
}
````

**2.获取连接对象**

````Golang
db, err := gorm.Open(mysql.New(mysql.Config{
 	DSN: "root:123456@tcp(127.0.0.1:3306)/demo?charset=utf8&parseTime=True&loc=Local", // DSN data source name
  	DefaultStringSize: 256, // string 类型字段的默认长度
  	DisableDatetimePrecision: true, // 禁用 datetime 精度，MySQL 5.6 之前的数据库不支持
  	DontSupportRenameIndex: true, // 重命名索引时采用删除并新建的方式，MySQL 5.7 之前的数据库和 MariaDB 不支持重命名索引
  	DontSupportRenameColumn: true, // 用 `change` 重命名列，MySQL 8 之前的数据库和 MariaDB 不支持重命名列
  	SkipInitializeWithVersion: false, // 根据当前 MySQL 版本自动配置
}), &gorm.Config{})
````

**3.新增、删除、修改SQL**

````Golang
// ----- insert-----
result := db.Create(&Student{Name: "张三",Age: 20}) // 此处不传指针对象也是OK的

// user.ID             返回插入数据的主键
// result.Error        返回 error
// result.RowsAffected 返回插入记录的条数

// ----- update -----
var student = &Student{
		Id:   1,
		Name: "李四",
		Age:  20,
}
// 全量更新，如果不带主键或者主键对应记录不存在，则是新增语句
db.Save(student) // UPDATE `t_student` SET `name`='李四',`age`=20,`image`='' WHERE `id` = 1

// 更新非零值字段
db.Updates(student) // UPDATE `t_student` SET `name`='李四',`age`=20 WHERE `id` = 1
// 使用map作为参数
db.Model(student).Where("id=?", 1).Updates(map[string]interface{}{
	"name": "李四",
})

// 更新指定列
db.Model(student).UpdateColumn("name", "李四")

// ----- delete -----
// 根据主键删除
db.Delete(student) // DELETE FROM `t_student` WHERE `t_student`.`id` = 1

// 根据自定义条件删除
db.Delete(&Student{}, "age>?", 20) // DELETE FROM `t_student` WHERE age>20
db.Delete(student, "age>?", 20) // DELETE FROM `t_student` WHERE age>20 AND `t_student`.`id` = 1
````

**4.查询SQL**

```Golang
// 列表查询
var list []Student
if err := db.Model(&Student{}).Where("id in ?", []int64{1, 2, 3}).Find(&list).Error; err != nil {
	panic(err)
}
fmt.Println(list)

// 单列查询
var student Student
// First、Last会排序，Take不会排序
err := db.Model(&Student{}).Where("id=?", 1).First(&student).Error
if err == gorm.ErrRecordNotFound {
	fmt.Println("记录不存在")
}
fmt.Println(student)

// 关联查询
var list2 []struct {
	Id         int64
	Name       string
	CourseName string
}
err = db.Model(&Student{}).
	Select("t_student.id,t_student.name,t_course.name as course_name").
	Joins("left join t_course on t_course.student_id=t_student.id").Find(&list2).Error
fmt.Println(list2)

// 分组和排序
err = db.Model(&Student{}).
	Select("age,max(id) as id").
	Where("age>?", 0).
	Group("age").
	Order("id desc").
	Find(&list).Error
fmt.Println(list)

// 查询某一列
var ids []int64
err = db.Model(&Student{}).
	Select("id").
	Pluck("id",&ids).Error
fmt.Println(ids)
```

**5.原生SQL**

```Golang
// Exec
err := db.Exec("update t_student set name='李四' where id=?", 1).Error
if err != nil {
	panic(err)
}

// Raw 类似原生Sql的Query
var list []Student
err = db.Raw("select * from t_student").Scan(&list).Error
fmt.Println(list)

// 获取原生的Rows对象
rows, err := db.Raw("select id from t_student").Rows()
if err != nil {
	panic(err)
}
defer rows.Close()
```




## 3.踩坑案例

**1.Scan与Find**

```Golang

```

**2.Update全量更新**

```Golang
// UPDATE `t_student` SET `name`='李四'
db.Model(&Student{}).Updates(map[string]interface{}{
	"name": "李四",
}).Where("id=?", 1)
```

**3.带ID的Create**

```Golang
var student = &Student{
		Id:   100000000000,
		Name: "李四",
		Age:  20,
}
// 可能会达到ID最大值，导致后续写入操作‘ON DUPLICATE KEY’错误
db.Create(student)
```

**4.使用原生SQL查询返回的Rows需要关闭**

```Golang
rows, err := db.Raw("select id from t_student").Rows()
if err != nil {
	panic(err)
}
// 不关闭会导致连接泄露
defer rows.Close()
```

**5.关联查询的表重命名**

```Golang
var list []struct {
	Id         int64
	Name       string
	CourseName string
}
// SELECT s.id,s.name,c.name as course_name FROM `t_student` as s left join t_course as c on c.student_id=s.id
db.Model(&Student{}).
	Select("s.id,s.name,c.name as course_name").
	Joins("as s left join t_course as c on c.student_id=s.id").Find(&list)
fmt.Println(list)
```



## 4.更多操作

**1.ON DUPLICATE KEY**

```Golang
// 在`id`冲突时，什么都不做
db.Clauses(clause.OnConflict{DoNothing: true}).Create(&user)

// 在`id`冲突时，将列更新为默认值
db.Clauses(clause.OnConflict{
  Columns:   []clause.Column{{Name: "id"}},
  DoUpdates: clause.Assignments(map[string]interface{}{"name": "","age":0, "sex": 1}),
}).Create(&user)

// 在`id`冲突时，将列更新为新值
db.Clauses(clause.OnConflict{
  Columns:   []clause.Column{{Name: "id"}},
  DoUpdates: clause.AssignmentColumns([]string{"name", "age", "sex", "phone"}),
}).Create(&user)

// 在冲突时，更新除主键以外的所有列到新值。
db.Clauses(clause.OnConflict{UpdateAll: true,}).Create(&user)
```



**2.Scopes复用**

```Golang
func TestScopes(t *testing.T) {
	var list []Student
    // SELECT * FROM `t_student` WHERE id>1 AND age > 18 ORDER BY id desc
	db.Scopes(AgeGreaterThan18, DescOrder).Where("id>?", 1).Find(&list)
	fmt.Println(list)
}

func AgeGreaterThan18(db *gorm.DB) *gorm.DB {
	return db.Where("age > ?", 18)
}

func DescOrder(db *gorm.DB) *gorm.DB {
	return db.Order("id desc")
}
```

**3.钩子函数和拦截器**

钩子函数：针对一个Model的创建、修改、删除可以做额外操作，共享一个事务；

```Golang
func (s *Student) BeforeCreate(tx *gorm.DB) (err error) {
	fmt.Println("BeforeCreate")
	s.Id = 0
	return
}

func (s *Student) AfterCreate(tx *gorm.DB) (err error) {
	fmt.Println("AfterCreate")
	return errors.New("bad Create")
}

func TestHook(t *testing.T) {
	var student = &Student{
		Id:   100000000,
		Name: "张三",
	}
	// INSERT INTO `t_student` (`name`,`age`,`image`) VALUES ('张三',0,'')
	db.Create(student)
    // 绕过model使用原生SQL不会执行钩子函数
    db.Exec("INSERT INTO `t_student` (`name`,`age`,`image`) VALUES ('张三',0,'')")
}
```

拦截器：针对所有的SQL语句可以做额外操作；

```Golang
_ = db.Callback().Create().After("gorm:create").Register("callback_name1", func(db *gorm.DB) {
	fmt.Println("Create After")
})
_ = db.Callback().Raw().After("gorm:create").Register("callback_name2", func(db *gorm.DB) {
	fmt.Println("Raw After")
})
var student = &Student{
	Id:   100000000,
	Name: "张三",
}
// 由于钩子函数返回error，数据写入被回滚，但拦截器执行了
db.Create(student)
// 原生SQL会触发Raw拦截器
db.Exec("INSERT INTO `t_student` (`name`,`age`) VALUES ('Bob',20)")
```

