var HomePage = PageView.extend({
    pageTitle: 'Player',
    initialize: function() {
        PageView.prototype.initialize.apply(this);
        PageView.prototype.render.apply(this);

        this._playerState = new PlayerState;
        this._playerView = new Player({
            el: this.$('[name=player]'),
            model: this._playerState
        });
        this._playerView.render();
        setInterval(this._playerState.fetch.bind(this._playerState), 1000);
        this._playerState.fetch();

        this._fileCollection = new FileCollection;
        this._fileCollection.fetch();
        this._browser = new Browser({
            model: this._fileCollection,
            el: this.$('[name=browser]')
        });
        this._browser.render();
    },
    play: function(file) {
        this._playerState.fetch({
            url: 'play/' + file
        });
    },
    template: $('#homepage-template').html(),
    render: function() {}
});

var PlayerState = RestModel.extend({
    url: 'state',
    defaults: {
        title: 'No track',
        time: 0,
        volume: 0,
        state: 'stopped' // One of playing, paused, stopped.
    },
    isNew: function() { return false; }
});

var File = RestModel.extend({
    defaults: {
        name: '',
        path: '',
        type: 'none'
    }
});

var FileCollection = RestCollection.extend({
    url: function() { return 'browse/' + this.path; },
    path: '',
    oldPath: '',
    model: File,
    parse: function() {
        var data = RestCollection.prototype.parse.apply(this, arguments);
        this.oldPath = this.path = data['path'];
        var files = data['files'];
        files.unshift({ name: '..', path: data['path'] + '/..', type: 'dir' });
        return files;
    },
    navigate: function(path) {
        this.path = this.oldPath + '/' + path;
        this.fetch();
    },
    comparator: function(f1, f2) {
        if(f1.get('type') == 'dir' && f2.get('type') == 'file')
            return -1;
        if(f1.get('type') == 'file' && f2.get('type') == 'dir')
            return 1;
        return (f1.get('name') < f2.get('name')) ?
            -1 :
            ((f1.get('name') == f2.get('name')) ? 0 : 1);
    }
});

var formatTime = function(seconds) {
    return Math.floor(seconds / 60) + ':' +
        ((seconds % 60 < 10) ? '0' : '') + (seconds % 60);
};

var Player = StaticView.extend({
    events: {
        'click button[name=backward]': function() {
            this.model.fetch({ url: '/back' });
        },
        'click button[name=stop]': function() {
            this.model.set('state', 'stopped');
            this.model.save();
        },
        'click button[name=playpause]': function() {
            console.log('state', this.model.get('state'));
            if(this.model.get('state') == 'playing')
                this.model.set('state', 'paused');
            else
                this.model.set('state', 'playing');
            this.model.save();
        },
        'click button[name=forward]': function() {
            this.model.fetch({ url: '/forward' });
        },
        'click button[name=next]': function() {
            this.model.fetch({ url: '/next' });
        },
        'click input[name=volume]': function() {
            this.model.set('volume', this.$('input[name=volume]:checked').val());
            this.model.save();
        }
    },
    template: $('#player-template').html(),
    templateParams: function() {
        var out = StaticView.prototype.templateParams.apply(this);
        _.extend(out, { 'formattedTime': formatTime(out['time']) });
        return out;
    }
});

var Browser = CollectionView.extend({
    initialize: function() {
        CollectionView.prototype.initialize.apply(this, arguments);
    },
    initializeView: function(view) {
        view.collection = this.model;
    },
    view: StaticView.extend({
        tagName: 'li',
        template: '\
        <%if(type == "dir") {%>\
        <svg viewBox="0 0 8 8" class="icon">\
            <use xlink:href="#folder" class="icon-folder"></use>\
        </svg>\
        <%} else {%>\
        <svg viewBox="0 0 8 8" class="icon">\
            <use xlink:href="#musical-note" class="icon-musical-note"></use>\
        </svg>\
        <%}%>\
        <%-name%>\
        ',
        events: {
            click: function() {
                if(this.model.get('type') == 'dir') {
                    this.collection.navigate(this.model.get('name'));
                } else {
                    gApplication.currentPage().play(this.model.get('path'))
                }
            }
        }
    })
});
