<?php
// +----------------------------------------------------------------------
// | ThinkPHP [ WE CAN DO IT JUST THINK ]
// +----------------------------------------------------------------------
// | Copyright (c) 2006~2017 http://thinkphp.cn All rights reserved.
// +----------------------------------------------------------------------
// | Licensed ( http://www.apache.org/licenses/LICENSE-2.0 )
// +----------------------------------------------------------------------
// | Author: liu21st <liu21st@gmail.com>
// +----------------------------------------------------------------------

namespace think\route;

use think\Facade;

abstract class Dispatch
{
    // 应用实例
    protected $app;
    // 调度操作
    protected $action;
    // 调度参数
    protected $param;
    // 状态码
    protected $code;
    // 是否区分大小写
    protected $caseUrl = true;

    public function __construct($action, $param = [], $code = null)
    {
        $this->app    = Facade::make('app');
        $this->action = $action;
        $this->param  = $param;
        $this->code   = $code;
    }

    public function caseUrl($case)
    {
        $this->caseUrl = $case;

        return $this;
    }

    abstract public function run();

}
