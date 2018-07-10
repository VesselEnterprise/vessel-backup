@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {
			$('.ui.form')
			  .form({
					/*
					on : 'blur',
					inline : true,
					*/
			    fields: {
			      first_name: {
			        identifier: 'first_name',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please enter your first name'
			          }
			        ]
			      },
			      last_name: {
			        identifier: 'last_name',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please enter your last name'
			          }
			        ]
			      },
			      email: {
			        identifier: 'email',
							inline: true,
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please provide an email address'
			          },
								{
			            type   : 'email',
			            prompt : 'Please enter a valid e-mail address'
			          }
			        ]
			      },
			      password: {
			        identifier: 'password',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please enter a password'
			          },
			          {
			            type   : 'minLength[6]',
			            prompt : 'Your password must be at least {ruleValue} characters'
			          }
			        ]
			      },
			      password_confirm: {
			        identifier: 'password_confirmation',
							depends: 'password',
			        rules: [
								{
			            type   : 'empty',
			            prompt : 'Please confirm your password'
			          },
								{
			            type   : 'minLength[6]',
			            prompt : 'Your password must be at least {ruleValue} characters'
			          },
								{
			            type   : 'match[password]',
			            prompt : 'Please make sure your password matches'
			          }
			        ]
			      },
						terms : {
							rules : [
								{
									type : 'checked',
									prompt : 'Please agree to the terms and conditions'
								}
							]
						}
			    }
			  })
			;
		});
	</script>
@endsection

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		Register
		<div class="sub header">Register your account to access the management portal</div>
	</h2>

	<form method="POST" action="{{ route('register') }}" class="ui form segment">
		@csrf
	  <div class="field">
	    <label>First Name</label>
	    <input type="text" id="first_name" name="first_name" placeholder="First Name" value="{{ old('first_name') }}" required autofocus>
	  </div>
	  <div class="field">
	    <label>Last Name</label>
	    <input type="text" id="last_name" name="last_name" placeholder="Last Name" value="{{ old('last_name') }}" required>
	  </div>
		<div class="field">
	    <label>E-Mail</label>
	    <input type="text" id="email" name="email" placeholder="john.doe@email.com" value="{{ old('email') }}" required>
	  </div>
		<div class="field">
	    <label>Password</label>
	    <input type="password" id="password" name="password" autocomplete="off" style="background-image: url(&quot;data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR4nGP6zwAAAgcBApocMXEAAAAASUVORK5CYII=&quot;); cursor: auto;" required>
	  </div>
		<div class="field">
	    <label>Confirm Password</label>
	    <input type="password" id="password-confirm" name="password_confirmation" style="background-image: url(&quot;data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR4nGP6zwAAAgcBApocMXEAAAAASUVORK5CYII=&quot;); cursor: auto;" autocomplete="off" required>
	  </div>
	  <div class="field">
	    <div class="ui checkbox">
	      <input type="checkbox" name="terms" tabindex="0">
	      <label>I agree to the Terms and Conditions</label>
	    </div>
	  </div>
		<div class="ui error message"></div>
	  <div class="ui primary submit button">{{ __('Register') }}</div>
	</form>

</div>
@endsection

<!--
<div class="container">
    <div class="row justify-content-center">
        <div class="col-md-8">
            <div class="card">
                <div class="card-header">{{ __('Register') }}</div>

                <div class="card-body">
                    <form method="POST" action="{{ route('register') }}" aria-label="{{ __('Register') }}">
                        @csrf

                        <div class="form-group row">
                            <label for="name" class="col-md-4 col-form-label text-md-right">{{ __('Name') }}</label>

                            <div class="col-md-6">
                                <input id="name" type="text" class="form-control{{ $errors->has('name') ? ' is-invalid' : '' }}" name="name" value="{{ old('name') }}" required autofocus>

                                @if ($errors->has('name'))
                                    <span class="invalid-feedback" role="alert">
                                        <strong>{{ $errors->first('name') }}</strong>
                                    </span>
                                @endif
                            </div>
                        </div>

                        <div class="form-group row">
                            <label for="email" class="col-md-4 col-form-label text-md-right">{{ __('E-Mail Address') }}</label>

                            <div class="col-md-6">
                                <input id="email" type="email" class="form-control{{ $errors->has('email') ? ' is-invalid' : '' }}" name="email" value="{{ old('email') }}" required>

                                @if ($errors->has('email'))
                                    <span class="invalid-feedback" role="alert">
                                        <strong>{{ $errors->first('email') }}</strong>
                                    </span>
                                @endif
                            </div>
                        </div>

                        <div class="form-group row">
                            <label for="password" class="col-md-4 col-form-label text-md-right">{{ __('Password') }}</label>

                            <div class="col-md-6">
                                <input id="password" type="password" class="form-control{{ $errors->has('password') ? ' is-invalid' : '' }}" name="password" required>

                                @if ($errors->has('password'))
                                    <span class="invalid-feedback" role="alert">
                                        <strong>{{ $errors->first('password') }}</strong>
                                    </span>
                                @endif
                            </div>
                        </div>

                        <div class="form-group row">
                            <label for="password-confirm" class="col-md-4 col-form-label text-md-right">{{ __('Confirm Password') }}</label>

                            <div class="col-md-6">
                                <input id="password-confirm" type="password" class="form-control" name="password_confirmation" required>
                            </div>
                        </div>

                        <div class="form-group row mb-0">
                            <div class="col-md-6 offset-md-4">
                                <button type="submit" class="btn btn-primary">
                                    {{ __('Register') }}
                                </button>
                            </div>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    </div>
</div>
-->
