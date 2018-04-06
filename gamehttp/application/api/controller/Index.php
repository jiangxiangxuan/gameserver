<?php
namespace app\api\controller;

use think\Controller;
use think\facade\Cache;
use think\facade\Session;

use app\api\model;
use app\api\model\GameUser;

class Index extends Controller
{
    public function index()
    {
		$reslut = array();
		$reslut['state'] = 1;
		$reslut['data']="参数错误";
        return json_encode($reslut,320);
    }
}
