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
        return "";
    }

	public function _empty()
	{
		$reslut          = array();
		$reslut['state'] = 1;
		$reslut['data']  = "参数错误";
        return json_encode( $reslut, 320 );
	}

	// 登录
    public function login( $username, $userpasswd )
    {
		$reslut   = array();
		$user     = new GameUser();
		$userinfo = $user->getUserInfoByUserName($username);
		if(count($userinfo) == 0 || $userinfo[0]["userpasswd"] != $userpasswd )
		{
			$reslut['state'] = 1;
			$reslut['data']  = "登录失败";
            return json_encode($reslut,320);
		}
		
		$token    = $this->token();
		$tokenkey = sprintf(Config::get('CacheKey.platform_token'), $token);		
		Cache::set($tokenkey,$userinfo[0]["uid"]);
		
		$userinfokey = sprintf(Config::get('CacheKey.platform_userinfo'), $userinfo[0]["uid"]);
		Cache::set($userinfokey,json_encode($userinfo[0]));

		$reslut['state']   = 0;
		$reslut['token']   = $token;
		$reslut["gateway"] = $this->getGateway();
		return json_encode($reslut);
    }

	// 验证token
	public function verifytoken( $token )
	{		
		$reslut   = array();
		
		$tokenkey = sprintf(Config::get('CacheKey.platform_token'),$token);
		$userid   = Cache::get($tokenkey);
		
		$userinfokey = sprintf(Config::get('CacheKey.platform_userinfo'), $userid);
		$user        = Cache::get($userinfokey);

		if(!$user)
		{
			$reslut['state'] = 1;
			$reslut['data']  = "验证失败";
            return json_encode($reslut,320);
		}
		else
		{		
			$reslut['state'] = 0;
			$reslut['data']  = $user;
			return json_encode($reslut);
		}		
	}

	//更新网关信息
    public function updategateway( $data )
    {
        Cache::set(Config::get("CacheKey.platform_gataways"),$data);
	}

    //获取网关信息
    private function getGateway()
	{
        $gatewayInfoListstr = Cache::get(Config::get("CacheKey.platform_gataways"));
        $gatewayInfoList    = json_decode($gatewayInfoListstr,true);
        $gatewayaddr        = "";
        $minUserCount       = 0;
        for ( $i=0; $i<count($gatewayInfoList); $i++ )
        {
            if( $minUserCount==0 )
			{
                $minUserCount=$gatewayInfoList[$i]['usercount'];
                $gatewayaddr=$gatewayInfoList[$i]['addr'];
            }
            if( $gatewayInfoList[$i]['usercount'] < $minUserCount)
			{
                $minUserCount=$gatewayInfoList[$i]['usercount'];
                $gatewayaddr=$gatewayInfoList[$i]['addr'];
            }
        }
        return $gatewayaddr;
    }
}
