<?php
namespace app\api\controller;

use think\facade\Cache;
use think\facade\Session;
use think\facade\Config;

use app\api\model;
use app\api\model\GameUser;

class Platform extends Base
{
    public function index()
    {
        return '';
    }
	
	public function _empty()
	{
		$reslut = array();
		$reslut['state'] = 1;
		$reslut['data']="参数错误";
        return json_encode($reslut,320);
	}
	
	// 登录
    public function login($username,$userpasswd)
    {
		$reslut = array();
		$user = new GameUser();
		$userinfo = $user->getUserInfoByUserName($username);
		if(count($userinfo) == 0 )
		{
			$reslut['state'] = 1;
			$reslut['data']="登录失败";
            return json_encode($reslut,320);
		}
		
		$tokenkey = sprintf(Config::get('CacheKey.platform_token'),'user'.$userinfo[0]["uid"]);
		
		Cache::set($tokenkey,json_encode($userinfo[0]));

		$reslut['state'] = 0;
		$reslut['data'] = $userinfo[0];
		return json_encode($reslut);
    }
	
	// 验证token
	public function verifytoken($token)
	{
		$tokenkey = sprintf(Config::get('CacheKey.platform_token'),$token);
		
		$reslut = array();
		$user = Cache::get($tokenkey);
		if(!$user)
		{
			$reslut['state'] = 1;
			$reslut['data']="验证失败";
            return json_encode($reslut,320);
		}
		else
		{		
			$reslut['state'] = 0;
			$reslut['data'] = $user;
			return json_encode($reslut);
		}		
	}
}
