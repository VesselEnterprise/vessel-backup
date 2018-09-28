@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});

			$('.menu .item').tab();

			var list_form = $('#list_form');

			$('table').tablesort();

			$('#delete_selected').click(function() {
				list_form.attr('action', '{{ route('storage.provider.destroyMultiple') }}');
				list_form.submit();
			});

			$('#create_provider').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ url('storage/create') }}';
			});

		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">

		@if(isset($success))
		<div class="ui positive message">
			<i class="close icon"></i>
			<div class="header">
				{{ $success }}
			</div>
		</div>
		@endif

		@if(isset($error))
		<div class="ui negative message">
			<i class="close icon"></i>
			<div class="header">
				{{ $error }}
			</div>
		</div>
		@endif

		<h2 class="ui header">
			Storage
			<div class="sub header">View and Manage Storage Providers</div>
		</h2>
	</div>

	<form id="list_form" action="" method="POST">
		@csrf

		<div class="ui very padded container">
			<table class="ui selectable sortable stackable padded striped table">
				<thead>
					<tr>
						<th class="no-sort" colspan="8">
			      	{{ $providers->links() }}
			    	</th>
				  </tr>
					<tr>
						<th class="no-sort"></th>
						<th>Name</th>
						<th>Status</th>
						<th>Description</th>
						<th>Type</th>
						<th>Server</th>
						<th>Bucket Name</th>
					</tr>
				</thead>
				<tbody>
					@foreach ($providers as $provider)
						<tr>
							<td>
								<div class="ui fitted checkbox">
				          <input name="selectedIds[]" type="checkbox" value="{{ $provider->uuid_text }}"><label></label>
				        </div>
							</td>
							<td><a href="{{ route('storage.show', $provider->uuid_text) }}">{{ $provider->provider_name }}</td>
							<td><span style="color: {{ $provider->active ? 'green' : 'red' }}">{{ $provider->active ? 'Active' : 'Disabled' }}</span></td>
							<td>{{ $provider->description }}</td>
							<td>{{ $provider->provider_type }}</td>
							<td>{{ $provider->server }}</td>
							<td>{{ $provider->bucket_name }}</td>
						</tr>
					@endforeach
				</tbody>
				<tfoot>
			    <tr>
						<th colspan="8">
							<div class="ui">
								<button id="create_provider" class="ui primary button">
									<i class="icon plus"></i>
								  Create New
								</button>
								<button id="delete_selected" class="red ui button">
									<i class="icon delete"></i>
								  Delete
								</button>
							</div>
				      {{ $providers->links() }}
			    	</th>
				  </tr>
				</tfoot>
			</table>
		</div>

	</form>

@endsection
