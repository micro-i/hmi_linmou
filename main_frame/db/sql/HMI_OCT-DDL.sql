CREATE TABLE IF NOT EXISTS product(
    `product_id` INT NOT NULL   COMMENT '产品ID;产品ID' ,
    `name` VARCHAR(255)    COMMENT '名称;名称' ,
    `en_name` VARCHAR(255)    COMMENT '英文名称;英文名称' ,
    `description` VARCHAR(900)    COMMENT '描述;产品描述' ,
    PRIMARY KEY (product_id)
)  COMMENT = '产品';

CREATE TABLE IF NOT EXISTS productgrade(
    `product_id` INT NOT NULL   COMMENT '产品ID' ,
    `defect_code` INT NOT NULL   COMMENT '缺陷码' ,
    `algorithm_name` VARCHAR(255) NOT NULL   COMMENT '算法名称' ,
    `customize_name` VARCHAR(255)    COMMENT '自定义名称' ,
    PRIMARY KEY (product_id,defect_code)
)  COMMENT = '产品缺陷';

CREATE TABLE IF NOT EXISTS product_classfied_result(
    `task_id` INT NOT NULL   COMMENT '任务号' ,
    `product_id` VARCHAR(255) NOT NULL   COMMENT '产品号' ,
    `workpiece_id` INT  COMMENT '工件号' ,
    `sn` VARCHAR(900)  COMMENT '产品条码' ,
    `comment` VARCHAR(900)    COMMENT '备注' ,
    `defect_code_list` VARCHAR(900) NOT NULL   COMMENT '缺陷码' ,
    `time` DATETIME NOT NULL   COMMENT '产品检测时间' ,
    PRIMARY KEY (task_id,workpiece_id,time)
)  COMMENT = '运行缺陷记录';

CREATE TABLE IF NOT EXISTS task(
    `task_id` INT NOT NULL   COMMENT '任务号' ,
    `product_id` INT NOT NULL   COMMENT '产品ID' ,
    `comment` VARCHAR(255) NOT NULL   COMMENT '批次号' ,
    `task_date` DATETIME NOT NULL   COMMENT '任务开始时间' ,
    `description` VARCHAR(900)    COMMENT '任务详情' ,
    PRIMARY KEY (task_id)
)  COMMENT = '任务列表';

CREATE TABLE IF NOT EXISTS product_model_inspect_result(
    `id` INT NOT NULL AUTO_INCREMENT  COMMENT '' ,
    `task_id` INT NOT NULL   COMMENT '任务号' ,
    `product_id` VARCHAR(255) NOT NULL   COMMENT '产品号' ,
    `workpiece_id` INT NOT NULL   COMMENT '工件号' ,
    `sn` VARCHAR(900) NOT NULL   COMMENT '产品条码' ,
    `comment` VARCHAR(900)    COMMENT '批次号' ,
    `defect_code` VARCHAR(900) NOT NULL   COMMENT '缺陷码' ,
    `width` VARCHAR(255)    COMMENT '块宽度' ,
    `height` VARCHAR(255)    COMMENT '块高度' ,
    `depth` VARCHAR(255)    COMMENT '块深度' ,
    `width_id` VARCHAR(255)    COMMENT '块宽序号' ,
    `height_id` VARCHAR(255)    COMMENT '块高序号' ,
    `depth_id` VARCHAR(255)    COMMENT '块深序号' ,
    `dtype` VARCHAR(255)    COMMENT '块采样类型' ,
    `time` DATETIME NOT NULL   COMMENT '块检测时间' ,
    `x_min` DECIMAL(24,6)    COMMENT '缺陷块坐标' ,
    `y_min` DECIMAL(24,6)    COMMENT '缺陷块坐标' ,
    `z_min` DECIMAL(24,6)    COMMENT '缺陷块坐标' ,
    `x_max` DECIMAL(24,6)    COMMENT '缺陷块坐标' ,
    `y_max` DECIMAL(24,6)    COMMENT '缺陷块坐标' ,
    `z_max` DECIMAL(24,6)    COMMENT '缺陷块坐标' ,
    `score` DECIMAL(24,6)    COMMENT '' ,
    `mask` VARCHAR(900)    COMMENT '备用' ,
    PRIMARY KEY (id,time)
)  COMMENT = '模型结果表';

CREATE TABLE IF NOT EXISTS user(
    `setting_name` VARCHAR(255) NOT NULL   COMMENT '设置名称' ,
    `setting_value` VARCHAR(255)    COMMENT '设置值' ,
    PRIMARY KEY (setting_name)
)  COMMENT = '用户配置';

