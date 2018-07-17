<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class AppSettingUser extends Model
{

		public function setting()
		{
			return $this->belongsTo('App\AppSetting', 'setting_id', 'setting_id');
		}

		public function user()
    {
        return $this->belongsTo('App\User', 'user_id', 'user_id');
    }

		protected $table = 'app_setting_user';
}
