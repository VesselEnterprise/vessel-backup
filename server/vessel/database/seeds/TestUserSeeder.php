<?php

use App\User;
use App\OAuthAccessToken;
use Illuminate\Database\Seeder;
use Spatie\BinaryUuid\HasBinaryUuid;

class TestUserSeeder extends Seeder
{

  use HasBinaryUuid;
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        $users = factory(User::class,100)->create();

        //Create personal access tokens
        foreach( $users as $user ) {

          //Create a dummy user Model
          $actor = new User;
          $actor->uuid = ''; //Avoid issues w/ Binary uuid / json serializations
          $token = $actor->createToken('Vessel');

          $tokenModel = OAuthAccessToken::find($token->token->id);
          $tokenModel->user_id = $user['uuid'];
          $tokenModel->save();

        }

    }
}
