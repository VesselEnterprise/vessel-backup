<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class AppSetting extends Model
{

		public function userSetting()
    {
        return $this->hasOne('App\AppSettingUser', 'setting_id', 'setting_id');
    }

    protected $table = 'app_setting';
		protected $hidden = ['created_at','updated_at','setting_id'];
}
